#include <stdio.h>
#include <string.h>

#include "osal.h"
#include "md5.h"
#include "debug.h"
#include "skhl_comm_define.h"
#include "skhl_comm_core.h"
#include "skhl_app_usr_config.h"

#define REPEAT_MAX_COUNT    10

enum
{
    USR_CONFIG_CHECK_LINK = 0,
    USR_CONFIG_SETTING,
    USR_CONFIG_WAIT_VERIFY,
    USR_CONFIG_DONE,
};

extern skhl_upgrade_ack usr_config_ack;

uint8_t target_rule = COMM_TARGET_ID_CENTER_BOARD;

static skhl_result skhl_send_data(skhl_local_pack_attr_t *attr)
{
    skhl_local_pack_attr_t pack;

    pack.cmd_set    = attr->cmd_set;
    pack.cmd_id     = attr->cmd_id;
    pack.cmd_dir    = attr->cmd_dir;
    pack.target     = attr->target;
    pack.seq_id     = attr->seq_id;
    pack.data       = attr->data;
    pack.data_len   = attr->data_len;
    pack.version    = attr->version;

    return skhl_comm_send_data(&pack);
}

skhl_result skhl_app_send_get_version(void)
{
    skhl_local_pack_attr_t pack_attr  = {0};
    static uint32_t seq_id = 0;

    pack_attr.cmd_set = CMD_SET_COMMON;
    pack_attr.cmd_dir = PACKAGE_DIR_REQ;
    pack_attr.cmd_id = CMD_ID_GET_VERSION;
    pack_attr.target = target_rule;
    pack_attr.seq_id = seq_id++;
    pack_attr.data   = NULL;
    pack_attr.data_len = 0;
    pack_attr.version  = COMM_PROTOCOL_V0;

    return skhl_send_data(&pack_attr);
}

skhl_result skhl_app_send_usr_setting(void *config)
{
    skhl_local_pack_attr_t pack_attr  = {0};
    static uint32_t seq_id = 0;
    usr_setting_req_t req = {0};
    usr_config_t *usr_config = config;

    memcpy(req.app_key, usr_config->usr_key, USER_KEY_LEN);
    memcpy(req.app_secret, usr_config->usr_secret, USER_SECRET_LEN);
    memcpy(req.device_id, usr_config->dev_id, USER_DEVICE_ID_LEN);
    memcpy(req.device_type, usr_config->dev_type, USER_DEVICE_TYPE_LEN);

    pack_attr.cmd_set = CMD_SET_COMMON;
    pack_attr.cmd_dir = PACKAGE_DIR_REQ;
    pack_attr.cmd_id = CMD_ID_USR_SETTING;
    pack_attr.target = target_rule;
    pack_attr.seq_id = seq_id++;
    pack_attr.data   = (uint8_t *)&req;
    pack_attr.data_len = sizeof(usr_setting_req_t);
    pack_attr.version  = COMM_PROTOCOL_V0;

    return skhl_send_data(&pack_attr);
}

skhl_result skhl_usr_config(void *cfg, uint8_t *result)
{
    uint8_t step                = USR_CONFIG_CHECK_LINK;
    uint32_t repeat_count       = 0;
    skhl_result usr_config_result  = 0;

    while (step != USR_CONFIG_DONE)
    {
        log_info("Configing...\n");
        switch (step)
        {
            case USR_CONFIG_CHECK_LINK:
                {
                    if (usr_config_ack.get_version_ack != TRUE)
                    {
                        skhl_app_send_get_version();
                        repeat_count++;
                        if (repeat_count > REPEAT_MAX_COUNT)
                        {
                            log_err("Error timeout when checking link with device!\n");
                            *result = ERROR_CODE_DEVICE_LINK;
                            usr_config_result = -1;
                            step = USR_CONFIG_DONE;
                        }
                    }
                    else
                    {
                        usr_config_ack.get_version_ack = FALSE;
                        repeat_count = 0;
                        step = USR_CONFIG_SETTING;
                    }
                }
                break;
            case USR_CONFIG_SETTING:
                if (usr_config_ack.usr_setting_ack != TRUE)
                {
                    skhl_app_send_usr_setting(cfg);
                    repeat_count++;
                    if (repeat_count > REPEAT_MAX_COUNT)
                    {
                        log_err("Error timeout when setting app key!\n");
                        *result = ERROR_CODE_SETTING_TIMEOUT;
                        usr_config_result = -1;
                        step = USR_CONFIG_DONE;
                    }
                }
                else
                {
                    usr_config_ack.usr_setting_ack = FALSE;
                    repeat_count = 0;
                    step = USR_CONFIG_WAIT_VERIFY;
                }
                break;
            case USR_CONFIG_WAIT_VERIFY:
                {
                    if (usr_config_ack.verify_ack != TRUE)
                    {
                        repeat_count++;
                        if (repeat_count > REPEAT_MAX_COUNT * 3)
                        {
                            *result = ERROR_CODE_AKAS_INVALID;
                            usr_config_result = -1;
                            step = USR_CONFIG_DONE;
                        }
                    }
                    else
                    {
                        usr_config_ack.verify_ack = FALSE;
                        repeat_count = 0;
                        usr_config_result = 0;
                        step = USR_CONFIG_DONE;
                    }
                }
                break;
            case USR_CONFIG_DONE:
            default:
                break;
        }

        skhl_sleep(1000);
    }

    return usr_config_result;
}

