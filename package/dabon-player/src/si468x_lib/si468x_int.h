#ifndef _SI468X_INT_H_
#define _SI468X_INT_H_

#define STATUS0_STCINT                0x01
#define STATUS0_DSRVINT                0x10
#define STATUS0_DACQINT                0x20
#define STATUS0_ERRCMD                0x40
#define STATUS0_CTS                    0x80
#define STATUS3_PUP_STATE_mask                0xC0
#define STATUS3_PUP_STATE_BOOTLOADER        0x80
#define STATUS3_PUP_STATE_APPLICATION        0xC0

struct si468x_status {
    uint8_t byte[4];
};

int si468x_wait_for_status0_bitmask(struct si468x_status* status, uint8_t bitmask);
int si468x_send_command_wait_cts_read_reply(uint8_t* cmd_data, uint32_t cmd_size,
                                            struct si468x_status* status,
                                            uint8_t* reply_data, uint32_t reply_size);

#endif //_SI468X_INT_H_
