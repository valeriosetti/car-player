#ifndef _SI468X_H_
#define _SI468X_H_

#include "stdint.h"
#include "si468x_utils.h"

#define SI468X_SUCCESS        0L
#define SI468X_ERROR        -1L

#define SI468X_PROP_INT_CTL_ENABLE                                0x0000
#define SI468X_PROP_DIGITAL_IO_OUTPUT_SELECT                        0x0200
#define SI468X_PROP_DIGITAL_IO_OUTPUT_SAMPLE_RATE                   0x0201
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT                        0x0202
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVERRIDES_1            0x0203
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVERRIDES_2            0x0204
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVERRIDES_3            0x0205
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVERRIDES_4            0x0206
#define SI468X_PROP_AUDIO_ANALOG_VOLUME                             0x0300
#define SI468X_PROP_AUDIO_MUTE                                      0x0301
#define SI468X_PROP_AUDIO_OUTPUT_CONFIG                             0x0302
#define SI468X_PROP_PIN_CONFIG_ENABLE                               0x0800
#define SI468X_PROP_WAKE_TONE_ENABLE                                0x0900
#define SI468X_PROP_WAKE_TONE_PERIOD                                0x0901
#define SI468X_PROP_WAKE_TONE_FREQ                                  0x0902
#define SI468X_PROP_WAKE_TONE_AMPLITUDE                             0x0903

enum si468x_exec_mode {
    EXEC_UNKNOWN_MODE,
    EXEC_BOOTLOADER_MODE,
    EXEC_FMHD_MODE,
    EXEC_DAB_MODE,
};

struct si468x_info {
    uint8_t chiprev;
    uint8_t romid;
    uint16_t part;
};

int si468x_powerup(void);
int si468x_load_init(void);
int si468x_host_load(uint8_t* img_data, uint32_t len);
int si468x_boot(void);
int si468x_get_part_info(struct si468x_info *info);
int si468x_get_sys_state(enum si468x_exec_mode* mode);
int si468x_set_property(uint16_t property, uint16_t value);
int si468x_get_property(uint16_t property, uint16_t* value);

#endif //_SI468X_H_
