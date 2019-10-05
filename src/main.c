#include <stdio.h>
#include <string.h>
#ifdef SK_WINDOWS
#include <stdlib.h>
#endif
#include <getopt.h>

#include "debug.h"
#include "osal.h"

#include "skhl_data_typedef.h"
#include "skhl_comm_define.h"
#include "skhl_comm_app.h"
#ifndef SK_WINDOWS
#include "skhl_app_upgrade.h"
#endif
#include "skhl_app_usr_config.h"

#define INPUT_BUFF_SIZE 128

uint32_t quit = 0;

int main(int32_t argc, char **argv)
{
    skhl_result         ret             = 0;
    comm_user_config_t  config          = {0};
    usr_config_t        usr_config      = {0};
    uint8_t             config_result   = 0;
    char input_com[INPUT_BUFF_SIZE]     = {0};
    char input_app_key[INPUT_BUFF_SIZE] = {0};
    char input_app_sec[INPUT_BUFF_SIZE] = {0};

INPUT_PORT:
    log_info("Please input serial port (COM0/1/2/3/4...): ");
    fflush(stdin);
    scanf("%s", input_com);

    if ((strncmp(input_com, "COM", 3) == 0) && (strlen(input_com) > 3))
    {
        log_info("Serial port: %s\n", input_com);
        config.port = input_com;
    }
    else
    {
        log_err("Input require 'COM?' but input is %s\n", input_com);
        goto INPUT_PORT;
    }

INPUT_APP_KEY:
    log_info("Please input APP KEY: ");
    fflush(stdin);
    scanf("%s", input_app_key);

    if (strlen(input_app_key) == USER_KEY_LEN)
    {
        log_info("App key: %s\n", input_app_key);
        strncpy(usr_config.usr_key, input_app_key, USER_KEY_LEN);
    }
    else
    {
        log_err("Input require %d char, but input length is %d\n", USER_KEY_LEN,
                                                            (int)strlen(input_app_key));
        goto INPUT_APP_KEY;
    }

INPUT_APP_SECRET:
    log_info("Please input APP SECRET: ");
    fflush(stdin);
    scanf("%s", input_app_sec);

    if (strlen(input_app_sec) == USER_SECRET_LEN)
    {
        log_info("App secret: %s\n", input_app_sec);
        strncpy(usr_config.usr_secret, input_app_sec, USER_SECRET_LEN);
    }
    else
    {
        log_err("Input require %d char, but input length is %d\n", USER_SECRET_LEN,
                                                            (int)strlen(input_app_sec));
        goto INPUT_APP_SECRET;
    }


    // start to setup conig.
    log_info("Start to initial communication with sk receiver...\n");
    config.rule = COMM_TARGET_ID_PC;
    ret = skhl_comm_init((void *)&config);
    if (ret != 0)
    {
        log_err("comm init err!\n");
        goto COMM_INIT_ERR;
    }

    ret = skhl_usr_config(&usr_config, &config_result);
    if (ret != 0)
    {
        log_err("usr config err!\n");
        log_err("App key & App secret config Failure!\n");
        log_err("    Error code = (0x%x)\n", config_result);
    }
    else
    {
        log_info("App key & App secret config success!\n");
        log_info("You can disconnect and reboot the device Now!\n");
    }

    quit = 1;
    skhl_comm_destory();

    system("pause");
    return 0;

COMM_INIT_ERR:

    return -1;
}

