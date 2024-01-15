#include "common.h"
#include "dht11.h"
#include "tfmini.h"
#include "device_queue.h"

#define SUPPORTED_DEVICES_COUNT 2

#define CP210X_VENDOR_ID 0x10C4
#define CP210X_PRODUCT_ID_1 0xEA60
#define CP210X_PRODUCT_ID_2 0xEA70

static result_codes is_supported_device(struct usb_serial_port *port, uint8_t *tag);
static int scann_usb_device(struct usb_device *dev, void *data);
static int usb_device_event(struct notifier_block *nb, unsigned long action, void *data);
static void remove_device(struct usb_serial_port *port, uint8_t tag);
static void add_new_device(struct usb_serial_port *port, uint8_t tag);
static int sensor_open(struct inode *inode, struct file *file);
static ssize_t sensor_read(struct file *file, char __user *buffer, size_t length, loff_t *offset);

static struct notifier_block usb_device_nb = {
    .notifier_call = usb_device_event,
};

static struct file_operations device_fops = {
    .owner = THIS_MODULE,
    .read = sensor_read,
    .open = sensor_open
};

struct sensors_t
{
    const char* name;
    uint8_t tag;
    struct class *device_class;
    result_codes (*device_tester)(struct usb_serial_port*, uint8_t*);
    struct device** (*device_adder)(struct class*, uint16_t, uint8_t*);
    result_codes (*device_reader)(struct usb_serial_port*, const char*, char*);
    ssize_t (*device_read_required_size)(const char*);
    uint16_t (*device_get_new_id)(void);
};

static struct sensors_t supported_sensors[SUPPORTED_DEVICES_COUNT] = {
    {
        .name = DHT11_NAME,
        .tag = DHT11_TAG,
        .device_tester = device_is_dht11,
        .device_adder = device_add_dht11,
        .device_reader = device_read_dht11,
        .device_read_required_size = device_read_required_size_dht11,
        .device_get_new_id = device_dht11_get_new_id
    },
    {
        .name = TFMINI_NAME,
        .tag = TFMINI_TAG,
        .device_tester = device_is_tfmini,
        .device_adder = device_add_tfmini,
        .device_reader = device_read_tfmini,
        .device_read_required_size = device_read_required_size_tfmini,
        .device_get_new_id = device_tfmini_get_new_id
    }
};

static int sensor_open(struct inode *inode, struct file *file)
{
    struct usb_serial_port* port = device_queue_get_port_by_dev(inode->i_rdev);

    if(!port)
        return -ENODEV;

    file->private_data = port;

    return 0;
}

static ssize_t sensor_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    struct usb_serial_port *port = file->private_data;
    uint8_t tag = device_queue_get_tag(port);
    ssize_t required_size;
    result_codes result;
    char* read_buffer;

    if(*offset > 0)
        return 0;

    for(int i = 0; i < SUPPORTED_DEVICES_COUNT; i++)
    {
        if(supported_sensors[i].tag != tag)
            continue;

        required_size = supported_sensors[i].device_read_required_size(file->f_path.dentry->d_name.name);

        if(required_size < 0)
            return -EINVAL;

        read_buffer = kmalloc(required_size, GFP_KERNEL);

        if(!read_buffer)
            return -ENOMEM;

        result = supported_sensors[i].device_reader(port, file->f_path.dentry->d_name.name, read_buffer);

        if(result == RESULT_OK)
        {
            if(copy_to_user(buffer, read_buffer, strlen(read_buffer) + 1))
            {
                kfree(read_buffer);
                return -EFAULT;
            }

            (*offset) += strlen(read_buffer) + 1;

            kfree(read_buffer);
            
            return strlen(read_buffer) + 1;
        }
        else if(result == RESULT_FALSE)
        {
            kfree(read_buffer);
            return -EIO;
        }
        else
        {
            kfree(read_buffer);
            return -EINVAL;
        }
    }

    return -ENODEV;
}

static int scann_usb_device(struct usb_device *dev, void *data)
{
    struct usb_interface *interface;
    struct usb_serial *serial;
    struct usb_serial_port *port;
    uint8_t tag;

    if(dev->descriptor.idVendor != CP210X_VENDOR_ID || (dev->descriptor.idProduct != CP210X_PRODUCT_ID_1 && dev->descriptor.idProduct != CP210X_PRODUCT_ID_2))
        return 0;

    if(dev->actconfig == NULL)
    {
        pr_err("Failed to get active config: %s\n", dev->product);
        return 0;
    }

    for (int i = 0; i < dev->actconfig->desc.bNumInterfaces; i++) 
    {
        interface = dev->actconfig->interface[i];

        if(interface == NULL)
        {
            pr_err("Failed to get interface: %s\n", dev->product);
            continue;
        }

        serial = usb_get_intfdata(interface);

        if (!serial) 
        {
            pr_err("Failed to get serial: %s\n", dev->product);
            continue;
        }

        for (int j = 0; j < serial->num_ports; j++) 
        {
            port = serial->port[j];
            
            if(port == NULL)
            {
                pr_err("Failed to get port: %s\n", dev->product);
                continue;
            }

            if(is_supported_device(port, &tag) == RESULT_TRUE)
            {
                if(!device_queue_exist_element(port))
                    add_new_device(port, tag);
                else
                    device_queue_mark_accessed(port);
            }
        }
    }

    return 0;
}

static result_codes is_supported_device(struct usb_serial_port *port, uint8_t *tag)
{
    for(int i = 0; i < SUPPORTED_DEVICES_COUNT; i++)
        if(supported_sensors[i].device_tester(port, tag) == RESULT_TRUE)
            return RESULT_TRUE;

    return RESULT_FALSE;
}

static void add_new_device(struct usb_serial_port *port, uint8_t tag)
{
    uint16_t id;
    uint8_t devices_objects_count;
    struct device** devices_objects;
    struct cdev** cdevs;

    for(int i = 0; i < SUPPORTED_DEVICES_COUNT; i++)
    {
        if(supported_sensors[i].tag != tag)
            continue;
        
        id = supported_sensors[i].device_get_new_id();
        devices_objects = supported_sensors[i].device_adder(supported_sensors[i].device_class, id, &devices_objects_count);
        
        cdevs = kmalloc(devices_objects_count * sizeof(struct cdev*), GFP_KERNEL);

        for (int i = 0; i < devices_objects_count; i++)
        {
            cdevs[i] = kmalloc(sizeof(struct cdev), GFP_KERNEL);

            cdev_init(cdevs[i], &device_fops);

            cdevs[i]->owner = THIS_MODULE;
            cdevs[i]->ops = &device_fops;

            cdev_add(cdevs[i], devices_objects[i]->devt, 1);
        }
        
        device_queue_add(port, tag, id, devices_objects, cdevs, devices_objects_count);

        pr_info("Device added: %s - %d\n", supported_sensors[i].name, id);
        
        break;
    }
}

static void remove_device(struct usb_serial_port *port, uint8_t tag)
{
    uint16_t id;
    uint8_t devices_objects_count;
    dev_t devt;
    struct device** devices_objects;
    struct cdev** cdevs;

    for(int i = 0; i < SUPPORTED_DEVICES_COUNT; i++)
    {
        if(supported_sensors[i].tag != tag)
            continue;

        id = device_queue_get_id(port);
        devices_objects_count = device_queue_get_devices_objects_count(port);
        devices_objects = device_queue_get_devices_objects(port);
        cdevs = device_queue_get_cdevs(port);
        devt = devices_objects[0]->devt;

        for (int j = 0; j < devices_objects_count; j++)
        {
            cdev_del(cdevs[j]);
            kfree(cdevs[j]);
            device_destroy(supported_sensors[i].device_class, devices_objects[j]->devt);
        }

        kfree(cdevs);
        kfree(devices_objects);

        unregister_chrdev_region(devt, devices_objects_count);

        device_queue_remove(port);

        pr_info("Device removed: %s - %d\n", supported_sensors[i].name, id);

        break;
    }
}

static int usb_device_event(struct notifier_block *nb, unsigned long action, void *data)
{
    struct usb_device *dev = (struct usb_device *)data;
    struct usb_serial_port *port; 

    if(action == USB_DEVICE_ADD)
        scann_usb_device(dev, NULL);  
    else if (action == USB_DEVICE_REMOVE)
    {
        usb_for_each_dev(NULL, scann_usb_device);

        while((port = device_queue_get_first_unaccessed_port()) != NULL)
            remove_device(port, device_queue_get_tag(port));

        device_queue_desmark_all_accessed();
    }

    return NOTIFY_OK;
}

static int __init drv_init(void)
{
    for(int i = 0; i < SUPPORTED_DEVICES_COUNT; i++)
    {
        supported_sensors[i].device_class = class_create(THIS_MODULE, supported_sensors[i].name);

        if (IS_ERR(supported_sensors[i].device_class))
        {
            printk(KERN_ALERT "Failed to create device class\n");
            return PTR_ERR(supported_sensors[i].device_class);
        }
    }

    usb_register_notify(&usb_device_nb);

    usb_for_each_dev(NULL, scann_usb_device);

    pr_info("CP210x sensors interfacing module initialized\n");
    
    return 0;
}

static void __exit drv_exit(void)
{
    struct usb_serial_port *port;

    usb_unregister_notify(&usb_device_nb);

    while(!device_queue_is_empty())
    {
        port = device_queue_get_last_node_port();

        remove_device(port, device_queue_get_tag(port));
    }

    for(int i = 0; i < SUPPORTED_DEVICES_COUNT; i++)
        class_destroy(supported_sensors[i].device_class);
        
    pr_info("CP210x sensors interfacing module exited\n");
}

module_init(drv_init);
module_exit(drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bottini, Franco Nicolas");
MODULE_AUTHOR("Robledo, Valentín");
MODULE_AUTHOR("Lencina, Aquiles Benjamin");
MODULE_DESCRIPTION("CP210x sensors interfacing driver");