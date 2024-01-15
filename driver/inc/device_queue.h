#ifndef __DEVICE_QUEUE_H__
#define __DEVICE_QUEUE_H__

#include "common.h"

int device_queue_is_empty(void);

int device_queue_exist_element(struct usb_serial_port *port);

void device_queue_add(struct usb_serial_port *port, uint8_t tag, uint16_t id, struct device** devices_objects, struct cdev **cdevs, uint8_t devices_objects_count);
void device_queue_remove(struct usb_serial_port *port);

int device_queue_get_devices_objects_count(struct usb_serial_port *port);
struct device** device_queue_get_devices_objects(struct usb_serial_port *port);
struct cdev** device_queue_get_cdevs(struct usb_serial_port *port);
uint16_t device_queue_get_id(struct usb_serial_port *port);
uint8_t device_queue_get_tag(struct usb_serial_port *port);

struct usb_serial_port* device_queue_get_last_node_port(void);
struct usb_serial_port* device_queue_get_port_by_dev(dev_t dev);
struct usb_serial_port* device_queue_get_first_unaccessed_port(void);

void device_queue_mark_accessed(struct usb_serial_port *port);
void device_queue_desmark_accessed(struct usb_serial_port *port);
void device_queue_desmark_all_accessed(void);

#endif // __DEVICE_QUEUE_H__