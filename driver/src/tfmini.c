#include "tfmini.h"

static const device_config_t config = {
    .baudrate = 115200,
    .bits_data = 8,
    .bits_parity = 0,
    .bits_stop = 1
};

static uint16_t next_id = 1;

static int is_valid_data(tfmini_data_t *data)
{
    uint16_t checksum = 0;
    
    for(int i = 0; i < sizeof(tfmini_data_t) - 1; i++)
        checksum += ((u8 *)data)[i];

    return (u8)checksum == data->checksum;
}

result_codes device_is_tfmini(struct usb_serial_port *port, uint8_t *tag)
{
    tfmini_data_t data;
    result_codes result = device_config_uart(port, config);

    if (result != RESULT_OK)
        return RESULT_FALSE;

    for(int i = 0; i < 10; i++)
    {
        result = device_read(port, &data, sizeof(tfmini_data_t));

        if (result != RESULT_OK)
            continue;
        
        if(!is_valid_data(&data))
            continue;

        if(data.tag != TFMINI_TAG || data.re_tag != TFMINI_TAG)
            continue;

        *tag = data.tag;

        return RESULT_TRUE;
    }
        
    return RESULT_FALSE;
}

result_codes device_read_tfmini(struct usb_serial_port *port, const char* device_name, char* buffer)
{
    tfmini_data_t data;
    result_codes result = device_config_uart(port, config);

    if (result != RESULT_OK)
        return DEVICE_READ_FAILED;

    while(1)
    {
        result = device_read(port, &data, sizeof(tfmini_data_t));

        if (result != RESULT_OK)
            continue;
        
        if(is_valid_data(&data))
            break;
    }

    if(strstr(device_name, "_distance"))
        sprintf(buffer, "%d\n", data.distance);
    else if(strstr(device_name, "_strength"))
        sprintf(buffer, "%d\n", data.strength);
    else
        return DEVICE_READ_FAILED;

    pr_info("%s: %s\n", device_name, buffer);

    return RESULT_OK;
}

ssize_t device_read_required_size_tfmini(const char* device_name)
{
    if(strstr(device_name, "_distance"))
        return snprintf(NULL, 0, "%d\n", 65535) + 1;
    else if(strstr(device_name, "_strength"))
        return snprintf(NULL, 0, "%d\n", 65535) + 1;
    else
        return DEVICE_READ_FAILED;
}

struct device** device_add_tfmini(struct class *device_class, uint16_t id, uint8_t *devices_objects_count)
{
    struct device **devices_objects = kmalloc(sizeof(struct devices*) * 2, GFP_KERNEL);
    dev_t dev;
    int ret;

    ret = alloc_chrdev_region(&dev, 0, 2, TFMINI_NAME);

    if (ret < 0) 
    {
        printk(KERN_ERR "Failed to allocate chrdev region\n");
        kfree(devices_objects);
        *devices_objects_count = 0;
        return NULL;
    }

    devices_objects[0] = device_create(device_class, NULL, dev, NULL, "%s_%d_distance", TFMINI_NAME, id);
    devices_objects[1] = device_create(device_class, NULL, dev + 1, NULL, "%s_%d_strength", TFMINI_NAME, id);

    if (!devices_objects[0] || !devices_objects[1]) 
    {
        printk(KERN_ERR "Failed to create device\n");
        unregister_chrdev_region(dev, 2);
        kfree(devices_objects);
        *devices_objects_count = 0;
        return NULL;
    }

    *devices_objects_count = 2;

    return devices_objects;
}

uint16_t device_tfmini_get_new_id(void)
{
    return next_id++;
}