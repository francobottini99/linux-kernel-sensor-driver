#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "common.h"
#include "cp210x.h"

typedef struct
{
    uint32_t baudrate;
    uint8_t bits_data;
    uint8_t bits_parity;
    uint8_t bits_stop;
} device_config_t;

result_codes device_config_uart(struct usb_serial_port *port, device_config_t config);

result_codes device_read(struct usb_serial_port *port, void* buffer, size_t buffer_size);

#endif // __DEVICE_H__