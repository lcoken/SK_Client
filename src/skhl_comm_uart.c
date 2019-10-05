#include <stdio.h>
#include <string.h>

#include <windows.h>

#include "debug.h"
#include "osal.h"

#include "skhl_data_typedef.h"
#include "skhl_comm_core.h"

static skhl_handle skhl_init_uart(comm_attr_t *desc)
{
    file_attr file = {0};
    skhl_handle comm_device;
    comm_attr_t *desc_attr = desc;

    if (desc_attr ==NULL)
    {
        log_err("error param!\n");
        return NULL;
    }

    file.name       = desc->name;
    file.access     = GENERIC_READ | GENERIC_WRITE;
    file.creation   = OPEN_EXISTING;
    file.flag       = FILE_ATTRIBUTE_NORMAL;
    file.share_mode = 0;
    comm_device = file_init(&file);
    if (NULL == comm_device)
    {
        log_err("file init %s error!\n", desc->name);
        return NULL;
    }
    log_debug("Success open %s\n", desc->name);

    SetupComm(comm_device, 1024, 1024);             // set input & output buffer to 1024

    // setting for read timeout
    desc_attr->attr.timeout.ReadIntervalTimeout            = 1; //10;
    desc_attr->attr.timeout.ReadTotalTimeoutMultiplier     = 1; //500;
    desc_attr->attr.timeout.ReadTotalTimeoutConstant       = 10; //5000;
    // setting for write timeout
    desc_attr->attr.timeout.WriteTotalTimeoutMultiplier    = 0; // 500;
    desc_attr->attr.timeout.WriteTotalTimeoutConstant      = 0; //2000;
    SetCommTimeouts(comm_device, &desc_attr->attr.timeout);
    // Serial Setting
    GetCommState(comm_device, &desc_attr->attr.dcb);
    desc_attr->attr.dcb.BaudRate                            = 115200;
    desc_attr->attr.dcb.ByteSize                            = 8;
    desc_attr->attr.dcb.Parity                              = NOPARITY;
    desc_attr->attr.dcb.StopBits                            = ONESTOPBIT;
    SetCommState(comm_device, &desc_attr->attr.dcb);

    // clean buffer.
    PurgeComm(comm_device, PURGE_TXCLEAR | PURGE_RXCLEAR);
//
//    SetCommMask(comm_device, EV_RXCHAR);

    return (skhl_handle)comm_device;
}

static uint32_t skhl_read_data(skhl_handle fd, uint8_t *buff, uint32_t size)
{
    int32_t real_size = 0;
    skhl_result ret = 0;

    ret = file_read(fd, buff, size, &real_size);
    if (ret < 0)
    {
        log_warn("file read error!\n");
        return 0;
    }

    return real_size;
}

static uint32_t skhl_write_data(skhl_handle fd, uint8_t *buff, uint32_t size)
{
    int32_t real_size = 0;
    skhl_result ret = 0;

    ret = file_write(fd, buff, size, &real_size);
    if (ret < 0)
    {
        log_warn("file write error!\n");
        return 0;
    }

    return real_size;
}

skhl_result skhl_close_uart(skhl_handle fd)
{
    file_close(fd);
    return 0;
}

skhl_opt_t uart_op = {
    .name       = "uart",
    .init       = skhl_init_uart,
    .read       = skhl_read_data,
    .write      = skhl_write_data,
    .destory    = skhl_close_uart,
};

skhl_result skhl_comm_uart_init(void)
{
    return skhl_register_comm_device(&uart_op);
}

skhl_result skhl_comm_uart_destory(void)
{
    return skhl_unregister_comm_device(&uart_op);
}

