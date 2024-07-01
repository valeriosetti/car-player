#ifndef _SI468X_H_
#define _SI468X_H_

#include "stdint.h"
#include "utils.h"

// Functions' return codes
#define SI468X_SUCCESS        0L
#define SI468X_ERROR        -1L

// List of common properties
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

// List of properties for DAB mode
#define SI468X_PROP_DAB_TUNE_FE_VARM                            0x1710
#define SI468X_PROP_DAB_TUNE_FE_VARB                            0x1711
#define SI468X_PROP_DAB_TUNE_FE_CFG                             0x1712
#define SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE                  0x8100
#define SI468X_PROP_DIGITAL_SERVICE_RESTART_DELAY               0x8101
#define SI468X_PROP_DAB_DIGRAD_INTERRUPT_SOURCE                 0xB000
#define SI468X_PROP_DAB_DIGRAD_RSSI_HIGH_THRESHOLD              0xB001
#define SI468X_PROP_DAB_DIGRAD_RSSI_LOW_THRESHOLD               0xB002
#define SI468X_PROP_DAB_VALID_RSSI_TIME                         0xB200
#define SI468X_PROP_DAB_VALID_RSSI_THRESHOLD                    0xB201
#define SI468X_PROP_DAB_VALID_ACQ_TIME                          0xB202
#define SI468X_PROP_DAB_VALID_SYNC_TIME                         0xB203
#define SI468X_PROP_DAB_VALID_DETECT_TIME                       0xB204
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE                  0xB300
#define SI468X_PROP_DAB_EVENT_MIN_SVRLIST_PERIOD                0xB301
#define SI468X_PROP_DAB_EVENT_MIN_SVRLIST_PERIOD_RECONFIG       0xB302
#define SI468X_PROP_DAB_EVENT_MIN_FREQINFO_PERIOD               0xB303
#define SI468X_PROP_DAB_XPAD_ENABLE                             0xB400
#define SI468X_PROP_DAB_DRC_OPTION                              0xB401
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_ENABLE                    0xB500
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGNAL_LEVEL_THRESHOLD    0xB501
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_WIN_THRESHOLD             0xB502
#define SI468X_PROP_DAB_CTRL_DAB_UNMUTE_WIN_THRESHOLD           0xB503
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGLOSS_THRESHOLD         0xB504
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGLOW_THRESHOLD          0xB505
#define SI468X_PROP_DAB_TEST_BER_CONFIG                         0xE800

// Typedefs
struct Si468x_status {
    uint8_t byte[4];
};

struct Si468x_info {
    uint8_t chiprev;
    uint8_t romid;
    uint16_t part;
};

struct Si468x_DAB_digrad_status {
    uint8_t hardmuteint:1;
    uint8_t ficerrint:1;
    uint8_t acqint:1;
    uint8_t rssihint:1;
    uint8_t rssilint:1;
    uint8_t hardmute:1;
    uint8_t ficerr:1;
    uint8_t acq:1;
    uint8_t valid:1;
    int8_t rssi;
    uint8_t snr;
    uint8_t fic_quality;
    uint8_t cnr;
    uint16_t FIB_error_count;
    uint32_t tune_freq;
    uint8_t tune_index;
    uint8_t tune_offet;
    int8_t fft_offset;
    uint16_t readantcap;
    uint16_t culevel;
    uint8_t fastdect;
};

struct Si468x_DAB_ensamble_info {
    uint16_t eid;
    char label[18];
    uint8_t ensamble_ecc;
    uint16_t char_abbrev;
};

struct Si468x_DAB_event_status {
    uint8_t recfgint:1;
    uint8_t recfgwrnint:1;
    uint8_t annoint:1;
    uint8_t freqinfoint:1;
    uint8_t svrlistint:1;
    uint8_t freq_info:1;
    uint8_t svrlist:1;
    uint16_t svrlistver;
};

struct Si468x_DAB_digital_service {
    uint32_t service_id;
    uint8_t service_info_1;
    uint8_t service_info_2;
    uint8_t service_info_3;
    uint8_t rsvd;
    char service_label[16];
};
#define get_service1_SrvLinkingInfoFlag(x)        ((x & BIT_MASK(6, 6)) >> 6)
#define get_service1_Pty(x)                        ((x & BIT_MASK(1, 5)) >> 1)
#define get_service1_PD_flag(x)                    ((x & BIT_MASK(0, 0)) >> 0)
#define get_service2_LOCAL_flag(x)                ((x & BIT_MASK(7, 7)) >> 7)
#define get_service2_CAId(x)                    ((x & BIT_MASK(4, 6)) >> 4)
#define get_service2_NUM_COMP(x)                ((x & BIT_MASK(0, 3)) >> 0)
#define get_service3_sicharset(x)                ((x & BIT_MASK(0, 3)) >> 0)

struct Si468x_DAB_digital_service_component {
    uint16_t component_id;
    uint8_t component_info;
    uint8_t valid_flags;
};

struct Si468x_DAB_digital_service_list_header {
    uint16_t service_list_size;
    uint16_t service_list_version;
    uint8_t num_of_services;
};

struct Si468x_DAB_time {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
};

enum DAB_audio_mode {
    DAB_AUDIO_MODE_DUAL,
    DAB_AUDIO_MODE_MONO,
    DAB_AUDIO_MODE_STEREO,
    DAB_AUDIO_MODE_JOIN_STEREO,
};

struct Si468x_DAB_audio_info {
    uint16_t bit_rate;
    uint16_t sample_rate;
    uint8_t drc_gain;
    enum DAB_audio_mode mode;
};

enum Si468x_exec_mode {
    EXEC_UNKNOWN_MODE,
    EXEC_BOOTLOADER_MODE,
    EXEC_FMHD_MODE,
    EXEC_DAB_MODE,
};

struct DAB_frequency {
    char* label;
    uint32_t frequency_kHz;
};

static struct DAB_frequency bandIII_DAB_frequency_list[] = {
    { "5A", 174928 },
    { "5B", 176640 },
    { "5C", 178352 },
    { "5D", 180064 },
    { "6A", 181936 },
    { "6B", 183648 },
    { "6C", 185360 },
    { "6D", 187072 },
    { "7A", 188928 },
    { "7B", 190640 },
    { "7C", 192352 },
    { "7D", 194064 },
    { "8A", 195936 },
    { "8B", 197648 },
    { "8C", 199360 },
    { "8D", 201072 },
    { "9A", 202928 },
    { "9B", 204640 },
    { "9C", 206352 },
    { "9D", 208064 },
    { "10A", 209936 },
    { "10N", 210096 },
    { "10B", 211648 },
    { "10C", 213360 },
    { "10D", 215072 },
    { "11A", 216928 },
    { "11N", 217088 },
    { "11B", 218640 },
    { "11C", 220352 },
    { "11D", 222064 },
    { "12A", 223936 },
    { "12N", 224096 },
    { "12B", 225648 },
    { "12C", 227360 },
    { "12D", 229072 },
    { "13A", 230784 },
    { "13B", 232496 },
    { "13C", 234208 },
    { "13D", 235776 },
    { "13E", 237488 },
    { "13F", 239200 },
};

// Common commands
int Si468x_powerup(void);
int Si468x_load_init(void);
int Si468x_host_load(uint8_t* img_data, uint32_t len);
int Si468x_boot(void);
int Si468x_get_part_info(struct Si468x_info *info);
int Si468x_get_sys_state(enum Si468x_exec_mode* mode);
//TODO: GET_POWER_UP_ARGS
//TODO: READ_OFFSET
//TODO: GET_FUNC_INFO
int Si468x_set_property(uint16_t property, uint16_t value);
int Si468x_get_property(uint16_t property, uint16_t* value);
int Si468x_start_digital_service(uint32_t service_id, uint32_t component_id);
//TODO: STOP_DIGITAL_SERVICE

// DAB commands
int Si468x_dab_get_digital_service_list(struct Si468x_DAB_digital_service_list_header* list_header, uint8_t** raw_service_list);
//TODO: Si468x_dab_get_digital_service_data
int Si468x_dab_tune_freq(uint8_t freq);
int Si468x_dab_digrad_status(uint8_t digrad_ack, uint8_t attune, uint8_t stc_ack, struct Si468x_DAB_digrad_status *status);
int Si468x_dab_get_event_status(uint8_t event_ack, struct Si468x_DAB_event_status* event_status);
    #define Si468x_KEEP_INT        0
    #define Si468x_CLEAR_INT    1
int Si468x_dab_get_ensamble_info(struct Si468x_DAB_ensamble_info* ensamble_info);
//TODO: Si468x_dab_get_service_linking_info
int Si468x_dab_get_freq_list(uint32_t **list, size_t *items_count);
int Si468x_dab_set_freq_list(void);
//TODO: Si468x_dab_get_component_info
int Si468x_dab_get_time(struct Si468x_DAB_time* dab_time);
int Si468x_dab_get_audio_info(struct Si468x_DAB_audio_info* audio_info);
//TODO: Si468x_dab_get_subchan_info
//TODO: Si468x_dab_get_freq_info
//TODO: Si468x_dab_get_service_info
//TODO: Si468x_dab_test_get_rssi
//TODO: Si468x_dab_test_get_ber_info

struct Si468x_FM_rsq_status {
    uint8_t snrhint:1;
    uint8_t snrlint:1;
    uint8_t rssihint:1;
    uint8_t rssilint:1;
    uint8_t bltf:1;
    uint8_t hddetected:1;
    uint8_t flt_hd_detected:1;
    uint8_t afcrl:1;
    uint8_t valid:1;
    uint16_t read_freq;
    int8_t freq_off;
    int8_t rssi;
    int8_t snr;
    uint8_t mult;
    uint16_t readantcap;
    uint8_t hdlevel;
    uint8_t filtered_hdlevel;
};

struct Si468x_FM_rds_status {
    uint8_t rdstpptyint:1;
    uint8_t rdspiint:1;
    uint8_t rdssyncint:1;
    uint8_t rdsfifoint:1;
    uint8_t tpptyvalid:1;
    uint8_t pivalid:1;
    uint8_t rdssync:1;
    uint8_t rdsfifolost:1;
    uint8_t tp:1;
    uint8_t pty:5;
    uint16_t pi;
    uint8_t rdsfifoused;
    uint8_t blea:2;
    uint8_t bleb:2;
    uint8_t blec:2;
    uint8_t bled:2;
    uint16_t blocka;
    uint16_t blockb;
    uint16_t blockc;
    uint16_t blockd;
};

struct Si468x_FM_rds_blockcount {
    uint16_t expected;
    uint16_t received;
    uint16_t uncorrectable;
};

// FM commands
int Si468x_fm_tune_freq(uint16_t freq);
int Si468x_fm_seek_start(uint8_t seek_up_down, uint8_t wrap);
    #define FM_SEEK_UP                    0x02
    #define FM_SEEK_DOWN                0x00
    #define FM_SEEK_WRAP                0x01
    #define FM_SEEK_NO_WRAP                0x00
int Si468x_fm_rsq_status(struct Si468x_FM_rsq_status* rsq_status);
//TODO: FM_ACF_STATUS
int Si468x_fm_rds_status(struct Si468x_FM_rds_status* rds_status);
int Si468x_fm_rds_blockcount(struct Si468x_FM_rds_blockcount* blockcount);
//TODO: GET_DIGITAL_SERVICE_LIST
//TODO: START_DIGITAL_SERVICE
//TODO: STOP_DIGITAL_SERVICE
//TODO: GET_DIGITAL_SERVICE_DATA
//TODO: HD_DIGRAD_STATUS
//TODO: HD_GET_EVENT_STATUS
//TODO: HD_GET_STATION_INFO
//TODO: HD_GET_PSD_DECODE
//TODO: HD_GET_ALERT_MSG
//TODO: HD_PLAY_ALERT_TONE
//TODO: HD_TEST_GET_BER_INFO
//TODO: HD_SET_ENABLED_PORTS
//TODO: HD_GET_ENABLED_PORTS
//TODO: TEST_GET_RSSI

#endif //_SI468X_H_
