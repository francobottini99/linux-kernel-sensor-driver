#include "device.h"

result_codes device_config_uart(struct usb_serial_port *port, device_config_t config)
{
    int status;
    u32 baudrate;
    u16 linectl;
    struct cp210x_flow_ctl flow_ctl;

    status = cp210x_open(port);

    if (status) 
    {
        pr_err("Failed to enable serial port: %d\n", status);
        return DEVICE_CONN_FAILED;
    }

    status = cp210x_set_baudrate(port, config.baudrate);

    if (status) 
    {
        pr_err("Failed to set serial baudrate: %d\n", status);
        cp210x_close(port);
        return DEVICE_CONN_FAILED;
    }

    status = cp210x_set_linectl(port, config.bits_data, config.bits_parity, config.bits_stop);

    if (status) 
    {
        pr_err("Failed to set serial line control: %d\n", status);
        cp210x_close(port);
        return DEVICE_CONN_FAILED;
    }

    status = cp210x_get_baudrate(port, &baudrate);

    if (status) 
    {
        pr_err("Failed to get serial baudrate: %d\n", status);
        cp210x_close(port);
        return DEVICE_CONN_FAILED;
    }

    status = cp210x_get_linectl(port, &linectl);

    if (status) 
    {
        pr_err("Failed to get serial line control: %d\n", status);
        cp210x_close(port);
        return DEVICE_CONN_FAILED;
    }

    status = cp210x_get_flowctl(port, &flow_ctl);

    if (status) 
    {
        pr_err("Failed to get serial flow control: %d\n", status);
        cp210x_close(port);
        return DEVICE_CONN_FAILED;
    }

    cp210x_close(port);

    return RESULT_OK;
}

result_codes device_read(struct usb_serial_port *port, void* buffer, size_t buffer_size)
{
    struct usb_serial *serial = port->serial;
    u8* transfer_buffer;
    int status;
    int bytes_read = 0;
    int bytes_remaining = buffer_size;

    status = cp210x_open(port);

    if (status) 
    {
        pr_err("Failed to enable serial port: %d\n", status);
        return DEVICE_CONN_FAILED;
    }

    transfer_buffer = kzalloc(buffer_size * sizeof(u8), GFP_KERNEL);

    if (!transfer_buffer) 
    {
        cp210x_close(port);
        pr_err("Failed to allocate transfer buffer\n");
        return DEVICE_CONN_FAILED;
    }

    status = usb_clear_halt(serial->dev, usb_rcvbulkpipe(serial->dev, port->bulk_in_endpointAddress));

    if (status) 
    {
        kfree(transfer_buffer);
        cp210x_close(port);
        pr_err("Failed to clear halt on bulk out endpoint: %d\n", status);
        return DEVICE_CONN_FAILED;
    }

    while (bytes_remaining > 0)
    {
        status = usb_bulk_msg(serial->dev, usb_rcvbulkpipe(serial->dev, port->bulk_in_endpointAddress), 
                              transfer_buffer + bytes_read, bytes_remaining, &bytes_read, 5000);

        if (status)
        {
            kfree(transfer_buffer);
            cp210x_close(port);
            return DEVICE_READ_FAILED;
        }

        bytes_remaining -= bytes_read;
    }

    for (int i = 0; i < buffer_size; i++)
        ((u8*)buffer)[i] = transfer_buffer[i];

    kfree(transfer_buffer);
    cp210x_close(port);

    return RESULT_OK;
}
