#ifndef _SI468X_DAB_H_
#define _SI468X_DAB_H_

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

struct si468x_DAB_digrad_status {
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

struct si468x_DAB_ensamble_info {
    uint16_t eid;
    char label[18];
    uint8_t ensamble_ecc;
    uint16_t char_abbrev;
};

struct si468x_DAB_event_status {
    uint8_t recfgint:1;
    uint8_t recfgwrnint:1;
    uint8_t annoint:1;
    uint8_t freqinfoint:1;
    uint8_t svrlistint:1;
    uint8_t freq_info:1;
    uint8_t svrlist:1;
    uint16_t svrlistver;
};

struct si468x_DAB_digital_service {
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

struct si468x_DAB_digital_service_component {
    uint16_t component_id;
    uint8_t component_info;
    uint8_t valid_flags;
};

struct si468x_DAB_digital_service_list_header {
    uint16_t service_list_size;
    uint16_t service_list_version;
    uint8_t num_of_services;
};

struct si468x_DAB_time {
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

struct si468x_DAB_audio_info {
    uint16_t bit_rate;
    uint16_t sample_rate;
    uint8_t drc_gain;
    enum DAB_audio_mode mode;
};

struct DAB_frequency {
    char* label;
    uint32_t frequency_kHz;
};

int si468x_dab_get_digital_service_list(struct si468x_DAB_digital_service_list_header* list_header, uint8_t** raw_service_list);
int si468x_dab_tune_freq(uint8_t freq);
int si468x_dab_digrad_status(uint8_t digrad_ack, uint8_t attune, uint8_t stc_ack, struct si468x_DAB_digrad_status *status);
int si468x_dab_get_event_status(uint8_t event_ack, struct si468x_DAB_event_status* event_status);
    #define si468x_KEEP_INT        0
    #define si468x_CLEAR_INT    1
int si468x_dab_get_ensamble_info(struct si468x_DAB_ensamble_info* ensamble_info);
int si468x_dab_get_freq_list(uint32_t **list, size_t *items_count);
int si468x_dab_set_freq_list(void);
int si468x_dab_get_time(struct si468x_DAB_time* dab_time);
int si468x_dab_get_audio_info(struct si468x_DAB_audio_info* audio_info);
int si468x_dab_start_digital_service(uint32_t service_id, uint32_t component_id);

#endif //_SI468X_DAB_H_
