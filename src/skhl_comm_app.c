#include <stdio.h>
#include <string.h>

#include <process.h>

#include "skhl_data_typedef.h"
#include "skhl_comm_define.h"
#include "skhl_comm_core.h"
#include "skhl_comm_uart.h"
#include "skhl_comm_app.h"

#include "debug.h"
#include "osal.h"
#include "md5.h"

static skhl_comm_router_t user_router[] = {
    {OPT_UART, COMM_TARGET_ID_PC},
    {OPT_UART, COMM_TARGET_ID_CENTER_BOARD}
};

skhl_upgrade_ack usr_config_ack = {0};

/*============================================================================*/
static skhl_result skhl_get_version(skhl_local_pack_attr_t *pack)
{
    skhl_result ret             = 0;
    skhl_local_pack_attr_t pack_attr  = {0};
    uint32_t version            = VERS;

    log_info("%s...\n", __func__);

    if (pack->cmd_dir == PACKAGE_DIR_ACK)
    {
        log_err("[Sk_receiver] Get version ack!\n");
        usr_config_ack.get_version_ack = TRUE;
    }
    else
    {
        pack_attr.cmd_set   = pack->cmd_set;
        pack_attr.cmd_id    = pack->cmd_id;
        pack_attr.target    = pack->source;
        pack_attr.seq_id    = pack->seq_id;
        pack_attr.data      = (uint8_t *)&version;
        pack_attr.data_len  = sizeof(uint32_t);
        pack_attr.cmd_dir   = PACKAGE_DIR_ACK;
        pack_attr.version   = COMM_PROTOCOL_V0;
        ret = skhl_comm_send_data(&pack_attr);
    }

    return ret;
}

static skhl_result skhl_usr_setting(skhl_local_pack_attr_t *pack)
{
    skhl_result ret             = 0;

    log_info("%s...\n", __func__);

    if (pack->cmd_dir == PACKAGE_DIR_ACK)
    {
        log_err("[Sk_receiver] Get usr setting ack!\n");
        usr_config_ack.usr_setting_ack = TRUE;
    }

    return ret;
}


static skhl_result skhl_config_wait_verify(skhl_local_pack_attr_t *pack)
{
    skhl_result ret             = 0;
    verify_result_ack_t *req    = (verify_result_ack_t *)pack->data;

    log_info("%s...\n", __func__);

    if (pack->cmd_dir == PACKAGE_DIR_ACK)
    {
        log_err("[Sk_receiver] Get verify result = (%d)!\n", req->verify_result);
        usr_config_ack.verify_ack = TRUE;
        usr_config_ack.verify_result = req->verify_result;
    }

    return ret;
}

static skhl_comm_item_t comm_cb[] =
{
    {CMD_SET_COMMON,    CMD_ID_GET_VERSION,     skhl_get_version},
    {CMD_SET_COMMON,    CMD_ID_USR_SETTING,     skhl_usr_setting},
    {CMD_SET_COMMON,    CMD_ID_WAIT_VERIFY,     skhl_config_wait_verify}
};


/*======================== Entry for Comm ===============================*/
skhl_result skhl_comm_init(void *usr_config)
{
    comm_user_config_t *config_local = usr_config;
    skhl_result             ret     = 0;
    comm_attr_t             attr    = {
        .name = config_local->port,
    };
    skhl_comm_core_config_t config  = {
        .cb             = comm_cb,
        .cb_size        = ARRAY_SIZE(comm_cb),
        .router         = user_router,
        .router_size    = ARRAY_SIZE(user_router),
        .this_host      = config_local->rule,
    };

    log_info("comm rule = 0x%x\n", config_local->rule);
    ret = skhl_comm_set_attr(OPT_UART, &attr);
    if (ret != 0)
    {
        log_err("set attr error!\n");
        goto ERR_SET_ATTR;
    }
    log_info("set port = %s\n", (int8_t *)config_local->port);

    ret = skhl_comm_uart_init();
    if (ret != 0)
    {
        log_err("comm device uart init failed!\n");
        goto ERR_COMM_UART_INIT;
    }

    ret = skhl_comm_core_init(&config);
    if (ret != 0)
    {
        log_err("comm core init failed!\n");
        skhl_comm_uart_destory();
        goto ERR_COMM_CORE_INIT;
    }

    log_debug("Success comm init!\n");

    return 0;

ERR_COMM_CORE_INIT:
    skhl_comm_uart_destory();
ERR_COMM_UART_INIT:
    skhl_comm_clear_attr(OPT_UART);
ERR_SET_ATTR:
    return ret;
}

skhl_result skhl_comm_destory(void)
{
    skhl_comm_core_destory();
    skhl_comm_uart_destory();
    return 0;
}

