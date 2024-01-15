#include "cp210x.h"

#define REQTYPE_HOST_TO_INTERFACE 0x41
#define REQTYPE_INTERFACE_TO_HOST 0xc1

#define CP210X_IFC_ENABLE	0x00
#define CP210X_SET_LINE_CTL	0x03
#define CP210X_GET_LINE_CTL	0x04
#define CP210X_GET_BAUDRATE	0x1D
#define CP210X_SET_BAUDRATE	0x1E
#define CP210X_SET_FLOW		0x13
#define CP210X_GET_FLOW		0x14

#define UART_ENABLE	 0x0001
#define UART_DISABLE 0x0000

#define BITS_DATA_5	0X0500
#define BITS_DATA_6	0X0600
#define BITS_DATA_7	0X0700
#define BITS_DATA_8	0X0800

#define BITS_PARITY_NONE  0x0000
#define BITS_PARITY_ODD	  0x0010
#define BITS_PARITY_EVEN  0x0020
#define BITS_PARITY_MARK  0x0030
#define BITS_PARITY_SPACE 0x0040

#define BITS_STOP_1	0x0000
#define BITS_STOP_2	0x0002

enum cp210x_event_state {
	ES_DATA,
	ES_ESCAPE,
	ES_LSR,
	ES_LSR_DATA_0,
	ES_LSR_DATA_1,
	ES_MSR
};

struct cp210x_port_private {
	u8			bInterfaceNumber;
	bool		event_mode;
	enum cp210x_event_state event_state;
	u8			lsr;

	struct mutex		mutex;
	bool			crtscts;
	bool			dtr;
	bool			rts;
};

static u16 cp210x_get_encode_linectl(u8 bits_data, u8 bits_parity, u8 bits_stop)
{
    u16 linectl = 0;

    switch (bits_data) {
        case 5:
            linectl |= BITS_DATA_5;
            break;
        case 6:
            linectl |= BITS_DATA_6;
            break;
        case 7:
            linectl |= BITS_DATA_7;
            break;
        case 8:
            linectl |= BITS_DATA_8;
            break;
        default:
            return -EINVAL;
    }

    switch (bits_parity) {
        case 0:
            linectl |= BITS_PARITY_NONE;
            break;
        case 1:
            linectl |= BITS_PARITY_ODD;
            break;
        case 2:
            linectl |= BITS_PARITY_EVEN;
            break;
        case 3:
            linectl |= BITS_PARITY_MARK;
            break;
        case 4:
            linectl |= BITS_PARITY_SPACE;
            break;
        default:
            return -EINVAL;
    }

    switch (bits_stop) {
        case 1:
            linectl |= BITS_STOP_1;
            break;
        case 2:
            linectl |= BITS_STOP_2;
            break;
        default:
            return -EINVAL;
    }

    return linectl;
}

static int cp210x_set_default_flowctl(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct cp210x_port_private *port_priv = usb_get_serial_port_data(port);
    struct cp210x_flow_ctl *flowctl = kmalloc(sizeof(struct cp210x_flow_ctl), GFP_KERNEL);
    int result;

    flowctl->ulXonLimit = 0;
	flowctl->ulXoffLimit = 0;
	flowctl->ulControlHandshake = 0;
	flowctl->ulFlowReplace = 0;

    result = usb_control_msg(serial->dev, usb_sndctrlpipe(serial->dev, 0), CP210X_SET_FLOW,
                             REQTYPE_HOST_TO_INTERFACE, 0, port_priv->bInterfaceNumber, flowctl,
                             sizeof(struct cp210x_flow_ctl), 5000);
    if (result < 0)
        return result;

    kfree(flowctl);

    return 0;
}

int cp210x_get_baudrate(struct usb_serial_port *port, u32 *buffer)
{
	struct usb_serial *serial = port->serial;
	struct cp210x_port_private *port_priv = usb_get_serial_port_data(port);
    u32 *current_baudrate = kmalloc(sizeof(u32), GFP_KERNEL);
    int result;

    result = usb_control_msg(serial->dev, usb_rcvctrlpipe(serial->dev, 0), CP210X_GET_BAUDRATE,
                             REQTYPE_INTERFACE_TO_HOST, 0, port_priv->bInterfaceNumber, current_baudrate,
                             sizeof(u32), 5000);
    if (result < 0)
        return result;

    memcpy(buffer, current_baudrate, sizeof(u32));

    kfree(current_baudrate);

    return 0;
}

int cp210x_set_baudrate(struct usb_serial_port *port, u32 baudrate)
{
	struct usb_serial *serial = port->serial;
	struct cp210x_port_private *port_priv = usb_get_serial_port_data(port);
    u32 *set_baudrate = kmalloc(sizeof(u32), GFP_KERNEL);
    int result;

    memcpy(set_baudrate, &baudrate, sizeof(u32));

    result = usb_control_msg(serial->dev, usb_sndctrlpipe(serial->dev, 0), CP210X_SET_BAUDRATE,
                             REQTYPE_HOST_TO_INTERFACE, 0, port_priv->bInterfaceNumber, set_baudrate, sizeof(u32), 5000);
    if (result < 0)
        return result;

    kfree(set_baudrate);

    return 0;
}

int cp210x_set_linectl(struct usb_serial_port *port, u8 bits_data, u8 bits_parity, u8 bits_stop)
{
	struct usb_serial *serial = port->serial;
	struct cp210x_port_private *port_priv = usb_get_serial_port_data(port);
    int result;
    u16 linectl;

    linectl = cp210x_get_encode_linectl(bits_data, bits_parity, bits_stop);

    if(linectl == -EINVAL)
        return -EINVAL;

    result = usb_control_msg(serial->dev, usb_sndctrlpipe(serial->dev, 0), CP210X_SET_LINE_CTL,
                             REQTYPE_HOST_TO_INTERFACE, linectl, port_priv->bInterfaceNumber, NULL, 0, 5000);
    if (result < 0)
        return result;

    return 0;
}

int cp210x_get_linectl(struct usb_serial_port *port, u16 *buffer)
{
	struct usb_serial *serial = port->serial;
	struct cp210x_port_private *port_priv = usb_get_serial_port_data(port);
    u16 *current_linectl = kmalloc(sizeof(u16), GFP_KERNEL);
    int result;

    result = usb_control_msg(serial->dev, usb_rcvctrlpipe(serial->dev, 0), CP210X_GET_LINE_CTL,
                             REQTYPE_INTERFACE_TO_HOST, 0, port_priv->bInterfaceNumber, current_linectl,
                             sizeof(current_linectl), 5000);
    if (result < 0)
        return result;

    memcpy(buffer, current_linectl, sizeof(u16));

    kfree(current_linectl);

    return 0;
}

int cp210x_get_flowctl(struct usb_serial_port *port, struct cp210x_flow_ctl *buffer)
{
	struct usb_serial *serial = port->serial;
	struct cp210x_port_private *port_priv = usb_get_serial_port_data(port);
    struct cp210x_flow_ctl *current_flowctl = kmalloc(sizeof(struct cp210x_flow_ctl), GFP_KERNEL);
    int result;

    result = usb_control_msg(serial->dev, usb_rcvctrlpipe(serial->dev, 0), CP210X_GET_FLOW,
                             REQTYPE_INTERFACE_TO_HOST, 0, port_priv->bInterfaceNumber, current_flowctl,
                             sizeof(struct cp210x_flow_ctl), 5000);
    if (result < 0)
        return result;

    current_flowctl->ulXonLimit = cpu_to_le32(current_flowctl->ulXonLimit);
	current_flowctl->ulXoffLimit = cpu_to_le32(current_flowctl->ulXoffLimit);
	current_flowctl->ulControlHandshake = cpu_to_le32(current_flowctl->ulControlHandshake);
	current_flowctl->ulFlowReplace = cpu_to_le32(current_flowctl->ulFlowReplace);

    memcpy(buffer, current_flowctl, sizeof(struct cp210x_flow_ctl));

    kfree(current_flowctl);

    return 0;
}

int cp210x_open(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct cp210x_port_private *port_priv = usb_get_serial_port_data(port);
    int result;

    result = usb_control_msg(serial->dev, usb_sndctrlpipe(serial->dev, 0), CP210X_IFC_ENABLE,
                             REQTYPE_HOST_TO_INTERFACE, UART_ENABLE, port_priv->bInterfaceNumber, NULL, 0, 5000);
    if (result < 0)
        return result;

    result = cp210x_set_default_flowctl(port);

    if (result < 0)
        return result;

    return 0;
}

int cp210x_close(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct cp210x_port_private *port_priv = usb_get_serial_port_data(port);
    int result;

    result = usb_control_msg(serial->dev, usb_sndctrlpipe(serial->dev, 0), CP210X_IFC_ENABLE,
                             REQTYPE_HOST_TO_INTERFACE, UART_DISABLE, port_priv->bInterfaceNumber, NULL, 0, 5000);
    if (result < 0)
        return result;

    return 0;
}