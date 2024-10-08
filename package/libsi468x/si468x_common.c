#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "si468x.h"
#include "si468x_int.h"
#include "si468x_platform.h"

// low level timings
#define WAIT_FOR_STATUS_BIT_POLLING_PERIOD_US        1000
#define WAIT_FOR_STATUS_BIT_MAX_TIMEOUT_MS            2000

#define SI468X_XTAL_FREQ                        19200000L
#define SI468X_XTAL_CL                            10    // in pF - that's taken from the crystal's datasheet!
#define SI468X_XTAL_STARTUP_BIAS_CURRENT        800    // in uA
                                                // See AN649 at section 9.1.5. It was assumed that:
                                                //     - CL=10pF
                                                //    - startup ESR = 5 x run ESR = 5 x 70ohm = 350ohm (max)

#define SI468X_CMD_RD_REPLY                                0x00
#define SI468X_CMD_POWER_UP                                0x01
#define SI468X_CMD_HOST_LOAD                               0x04
#define SI468X_CMD_FLASH_LOAD                              0x05
#define SI468X_CMD_LOAD_INIT                               0x06
#define SI468X_CMD_BOOT                                    0x07
#define SI468X_CMD_GET_PART_INFO                           0x08
#define SI468X_CMD_GET_SYS_STATE                           0x09
#define SI468X_CMD_GET_POWER_UP_ARGS                       0x0A
#define SI468X_CMD_READ_OFFSET                             0x10
#define SI468X_CMD_GET_FUNC_INFO                           0x12
#define SI468X_CMD_SET_PROPERTY                            0x13
#define SI468X_CMD_GET_PROPERTY                            0x14
#define SI468X_CMD_WRITE_STORAGE                           0x15
#define SI468X_CMD_READ_STORAGE                            0x16

#define MAX_FLASH_LOAD_PAYLOAD        2048
#define FLASH_LOAD_HEADER            4
static uint8_t dummy_buffer[MAX_FLASH_LOAD_PAYLOAD + FLASH_LOAD_HEADER];

int si468x_wait_for_status0_bitmask(struct si468x_status* status, uint8_t bitmask)
{
    struct timespec sys_time;
    long start_time_ms, curr_time_ms;

    clock_gettime(CLOCK_MONOTONIC, &sys_time);
    start_time_ms = sys_time.tv_sec * 1000 + sys_time.tv_nsec/1000000;

    do {
        si468x_read_reply(NULL, 0, status);
        clock_gettime(CLOCK_MONOTONIC, &sys_time);
        curr_time_ms = sys_time.tv_sec * 1000 + sys_time.tv_nsec/1000000;
        if (curr_time_ms - start_time_ms > WAIT_FOR_STATUS_BIT_MAX_TIMEOUT_MS) {
            ERROR("timeout");
            return -1;
        }
        usleep(WAIT_FOR_STATUS_BIT_POLLING_PERIOD_US);
    } while ((status->byte[0] & bitmask) != bitmask);

    return 0;
}

int si468x_send_command_wait_cts_read_reply(uint8_t* cmd_data, uint32_t cmd_size,
                                            struct si468x_status* status,
                                            uint8_t* reply_data, uint32_t reply_size)
{
    int ret;

    ret = si468x_send_command(cmd_data, cmd_size);
    if (ret < 0) {
        return ret;
    }

    ret = si468x_wait_for_status0_bitmask(status, STATUS0_CTS);
    if (ret < 0) {
        return ret;
    }
    if (reply_data != NULL) {
        ret = si468x_read_reply(reply_data, reply_size, status);
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

int si468x_powerup()
{
    uint8_t data_out[16] = {
        SI468X_CMD_POWER_UP,
        0x00,    // toggle interrupt when CTS is available
        0x1f,    // external crystal; TR_SIZE=0x7 (see AN649 at section 9.1)
        0x7f,    // see comments in the define
        0x00,
        0xf8,
        0x24,
        0x01,
        0x20,    // see AN649 at section 9.3,
        0x10,    // fixed
        0x00,    // fixed
        0x00,    // fixed
        0x00,    // fixed
        0x18,    // see AN649 at section 9.2
        0x00,    // fixed
        0x00,    // fixed
    };
    int ret_val;

    ret_val = si468x_send_command(data_out, sizeof(data_out));
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    struct si468x_status status;
    ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    // data_in[3] contains informations about the current device's state
    if ((status.byte[3] & STATUS3_PUP_STATE_mask) != STATUS3_PUP_STATE_BOOTLOADER) {
        ERROR_TRACE();
        DUMP_STATUS(status);
        return -1;
    }
    return 0;
}

int si468x_load_init()
{
    uint8_t data_out[2] = {
        SI468X_CMD_LOAD_INIT,
        0x00
    };
    int ret_val;

    ret_val = si468x_send_command(data_out, sizeof(data_out));
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    struct si468x_status status;
    ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);

    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }
    return 0;
}

int si468x_host_load(uint8_t* img_data, uint32_t len)
{
    uint32_t curr_len;
    int ret_val;
    struct si468x_status status;
    uint32_t sent_bytes = 0;

    dummy_buffer[0] = SI468X_CMD_HOST_LOAD;
    dummy_buffer[1] = 0x00;
    dummy_buffer[2] = 0x00;
    dummy_buffer[3] = 0x00;

    // Each command can send up to 4096 bytes. Therefore, if the sent
    // image is bigger, then split it into consecutive chucks
    do {
        curr_len = (len > MAX_FLASH_LOAD_PAYLOAD) ? MAX_FLASH_LOAD_PAYLOAD : len;
        // update only the payload of the command with the FW's content
        memcpy(&(dummy_buffer[4]), img_data, curr_len);
        ret_val = si468x_send_command(dummy_buffer, FLASH_LOAD_HEADER + curr_len);
        if (ret_val < 0) {
            ERROR_TRACE();
            return ret_val;
        }

        ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
        if (ret_val < 0) {
            ERROR_TRACE();
            return ret_val;
        }

        sent_bytes += curr_len;

        len -= curr_len;
        img_data += curr_len;
    } while(len > 0);

    return 0;
}

int si468x_boot()
{
    int ret_val;
    uint8_t data_out[2] = {
        SI468X_CMD_BOOT,
        0x00
    };

    ret_val = si468x_send_command(data_out, sizeof(data_out));
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    struct si468x_status status;
    ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    // data_in[3] contains informations about the current device's state
    if ((status.byte[3] & STATUS3_PUP_STATE_mask) != STATUS3_PUP_STATE_APPLICATION) {
        ERROR_TRACE();
        DUMP_STATUS(status);
        return -1;
    }
    return 0;
}

int si468x_get_part_info(struct si468x_info *info)
{
    uint8_t data_out[2] = {
        SI468X_CMD_GET_PART_INFO,
        0x00
    };
    si468x_send_command(data_out, sizeof(data_out));

    struct si468x_status status;
    int ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    uint8_t data_in[19];
    si468x_read_reply(data_in, sizeof(data_in), &status);

    if(status.byte[0] & (STATUS0_CTS | STATUS0_ERRCMD)) {
        info->chiprev = data_in[0];
        info->romid = data_in[1];
        info->part = ((uint16_t)data_in[5] << 8) | data_in[4];
    }
    return 0;
}

int si468x_get_sys_state(enum si468x_exec_mode* mode)
{
    uint8_t data_out[2] = {
        SI468X_CMD_GET_SYS_STATE,
        0x00
    };
    si468x_send_command(data_out, sizeof(data_out));

    struct si468x_status status;
    uint8_t data_in[2];
    int ret; 

    ret = si468x_read_reply(data_in, sizeof(data_in), &status);
    if (ret < 0) {
        return ret;
    }

    switch(status.byte[0]) {
        case 0:
            *mode = EXEC_BOOTLOADER_MODE;
            break;
        case 1:
            *mode = EXEC_FMHD_MODE;
            break;
        case 2:
            *mode = EXEC_DAB_MODE;
            break;
        default:
            *mode = EXEC_UNKNOWN_MODE;
            break;
    }

    return 0;
}

int si468x_get_property(uint16_t property, uint16_t* value)
{
    uint8_t data_out[6] = {
        SI468X_CMD_GET_PROPERTY,
        0x01,
        (uint8_t)(0x00FF & property),
        (uint8_t)((0xFF00 & property) >> 8),
    };
    int ret; 
    struct si468x_status status;
    uint8_t data_in[2];

    ret = si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status,
                                                    data_in, sizeof(data_in));
    if (ret < 0) {
        ERROR_TRACE();
        return ret;
    }

    *value = ((uint16_t)data_in[0] | ((uint16_t)(data_in[1]) << 8));

    return 0;
}

int si468x_set_property(uint16_t property, uint16_t value)
{
    uint8_t data_out[6] = {
        SI468X_CMD_SET_PROPERTY,
        0x00,
        (uint8_t)(0x00FF & property),
        (uint8_t)((0xFF00 & property) >> 8),
        (uint8_t)(0x00FF & value),
        (uint8_t)((0xFF00 & value) >> 8),
    };
    int ret; 
    struct si468x_status status;

    ret = si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, NULL, 0);
    if (ret < 0) {
        ERROR_TRACE();
        return ret;
    }

    return 0;
}
