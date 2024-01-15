#ifndef __DHT11_H__
#define __DHT11_H__

#include "common.h"
#include "device.h"

#define DHT11_TAG 0x73
#define DHT11_NAME "DHT11"

typedef struct __attribute__((packed))
{
    uint8_t tag;
    uint8_t re_tag;
    uint8_t temperature;
    uint8_t humidity;
    uint8_t heat_index;
    uint8_t checksum;
} dht11_data_t;

result_codes device_is_dht11(struct usb_serial_port *port, uint8_t *tag);

result_codes device_read_dht11(struct usb_serial_port *port, const char* device_name, char* buffer);

struct device** device_add_dht11(struct class *device_class, uint16_t id, uint8_t *devices_objects_count);

ssize_t device_read_required_size_dht11(const char* device_name);

uint16_t device_dht11_get_new_id(void);

#endif // __DHT11_H__