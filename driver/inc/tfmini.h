#ifndef __TFMINI_H__
#define __TFMINI_H__

#include "common.h"
#include "device.h"

#define TFMINI_TAG 0x59
#define TFMINI_NAME "TFMINI"

typedef struct __attribute__((packed))
{
    uint8_t  tag;
    uint8_t  re_tag;
    uint16_t distance;
    uint16_t strength;
    uint8_t  reserved;
    uint8_t  signal_quality;
    uint8_t  checksum;
} tfmini_data_t;

result_codes device_is_tfmini(struct usb_serial_port *port, uint8_t *tag);

result_codes device_read_tfmini(struct usb_serial_port *port, const char* device_name, char* buffer);

struct device** device_add_tfmini(struct class *device_class, uint16_t id, uint8_t *devices_objects_count);

uint16_t device_tfmini_get_new_id(void);

ssize_t device_read_required_size_tfmini(const char* device_name);

#endif // __TFMINI_H__