#ifndef __CP210X_H__
#define __CP210X_H__

#include "common.h"

struct cp210x_flow_ctl {
	__le32	ulControlHandshake;
	__le32	ulFlowReplace;
	__le32	ulXonLimit;
	__le32	ulXoffLimit;
};

int cp210x_get_baudrate(struct usb_serial_port *port, u32 *buffer);
int cp210x_set_baudrate(struct usb_serial_port *port, u32 baudrate);
int cp210x_set_linectl(struct usb_serial_port *port, u8 bits_data, u8 bits_parity, u8 bits_stop);
int cp210x_get_linectl(struct usb_serial_port *port, u16 *buffer);
int cp210x_get_flowctl(struct usb_serial_port *port, struct cp210x_flow_ctl *buffer);

int cp210x_open(struct usb_serial_port *port);
int cp210x_close(struct usb_serial_port *port);

#endif // __CP210X_H__