#ifndef _SI468X_FM_H_
#define _SI468X_FM_H_

struct si468x_FM_rsq_status {
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

struct si468x_FM_rds_status {
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

struct si468x_FM_rds_blockcount {
    uint16_t expected;
    uint16_t received;
    uint16_t uncorrectable;
};

int si468x_fm_tune_freq(uint16_t freq);
int si468x_fm_seek_start(uint8_t seek_up_down, uint8_t wrap);
    #define FM_SEEK_UP                    0x02
    #define FM_SEEK_DOWN                0x00
    #define FM_SEEK_WRAP                0x01
    #define FM_SEEK_NO_WRAP                0x00
int si468x_fm_rsq_status(struct si468x_FM_rsq_status* rsq_status);
int si468x_fm_rds_status(struct si468x_FM_rds_status* rds_status);
int si468x_fm_rds_blockcount(struct si468x_FM_rds_blockcount* blockcount);

#endif //_SI468X_FM_H_
