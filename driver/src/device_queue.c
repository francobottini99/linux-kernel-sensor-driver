#include "device_queue.h"

typedef struct node_t
{
    uint8_t access_flag;
    uint8_t tag;
    uint16_t id;
    uint8_t devices_objects_count;
    struct usb_serial_port *port;
    struct device **devices_objects;
    struct cdev **cdevs;
    struct node_t *next;
} node_t;

node_t* first_node = NULL;
node_t* last_node = NULL;

uint16_t device_count = 0;

static node_t* get_node(struct usb_serial_port *port)
{
    node_t* it;

    for (it = first_node; it; it = it->next)
        if (it->port == port)
            break;

    return it;
}

static node_t* get_node_parent(node_t* n)
{
    node_t *parent = first_node;

    if(!parent || n == parent)
        return NULL;

    while (parent->next != n)
        parent = parent->next;

    return parent;
}

static void remove_node(node_t* n)
{
    node_t *parent = get_node_parent(n);

    if(!parent)
        first_node = n->next;
    else if (!n->next)
        parent->next = NULL;  
    else 
        parent->next = n->next;

    if (n == last_node)
    {
        if(parent)
            last_node = parent;
        else
            last_node = first_node;
    }
    
    device_count--;

    kfree(n);
}

int device_queue_is_empty(void)
{
    return device_count == 0;
}

int device_queue_exist_element(struct usb_serial_port *port)
{
    return get_node(port) != NULL;
}

void device_queue_add(struct usb_serial_port *port, uint8_t tag, uint16_t id, struct device** devices_objects, struct cdev **cdevs, uint8_t devices_objects_count)
{
    node_t* new_node;

    new_node = (node_t*) kzalloc(sizeof(node_t), GFP_KERNEL);

    new_node->access_flag = 0;
    new_node->tag = tag;
    new_node->port = port;
    new_node->devices_objects = devices_objects;
    new_node->devices_objects_count = devices_objects_count;
    new_node->cdevs = cdevs;
    new_node->id = id;
    new_node->next = NULL;

    if(!first_node)
    {
        first_node = new_node;
        last_node = new_node;
    }
    else
    {
        last_node->next = new_node;
        last_node = new_node;
    }

    device_count++;
}

void device_queue_remove(struct usb_serial_port *port)
{
    node_t* n = get_node(port);

    if(n)
        remove_node(n);
}

int device_queue_get_devices_objects_count(struct usb_serial_port *port)
{
    node_t* n = get_node(port);

    if(n)
        return n->devices_objects_count;

    return -1;
}

struct device** device_queue_get_devices_objects(struct usb_serial_port *port)
{
    node_t* n = get_node(port);

    if(n)
        return n->devices_objects;

    return NULL;
}

struct cdev** device_queue_get_cdevs(struct usb_serial_port *port)
{
    node_t* n = get_node(port);

    if(n)
        return n->cdevs;

    return NULL;
}

struct usb_serial_port* device_queue_get_last_node_port(void)
{
    return last_node->port;
}

uint8_t device_queue_get_last_node_tag(void)
{
    return last_node->tag;
}

uint16_t device_queue_get_id(struct usb_serial_port *port)
{
    node_t* n = get_node(port);

    if(n)
        return n->id;

    return -1;
}

uint8_t device_queue_get_tag(struct usb_serial_port *port)
{
    node_t* n = get_node(port);

    if(n)
        return n->tag;

    return -1;
}

void device_queue_mark_accessed(struct usb_serial_port *port)
{
    node_t* n = get_node(port);

    if(n)
        n->access_flag = 1;
}

void device_queue_desmark_accessed(struct usb_serial_port *port)
{
    node_t* n = get_node(port);

    if(n)
        n->access_flag = 0;
}

void device_queue_desmark_all_accessed(void)
{
    node_t* it;

    for (it = first_node; it; it = it->next)
        it->access_flag = 0;
}

struct usb_serial_port* device_queue_get_first_unaccessed_port(void)
{
    node_t* it;

    for (it = first_node; it; it = it->next)
        if(!it->access_flag)
            return it->port;

    return NULL;
}

struct usb_serial_port* device_queue_get_port_by_dev(dev_t dev)
{
    node_t* it;

    for (it = first_node; it; it = it->next)
        for(int i = 0; i < it->devices_objects_count; i++)
            if(it->devices_objects[i]->devt == dev)
                return it->port;

    return NULL;
}