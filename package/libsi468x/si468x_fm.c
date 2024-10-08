#include "si468x.h"
#include "si468x_fm.h"
#include "si468x_int.h"
#include "si468x_platform.h"

// List of command for FM mode
#define SI468X_CMD_FM_TUNE_FREQ                 0x30
#define SI468X_CMD_FM_SEEK_START                0x31
#define SI468X_CMD_FM_RSQ_STATUS                0x32
#define SI468X_CMD_FM_RDS_STATUS                0x34
#define SI468X_CMD_FM_RDS_BLOCKCOUNT            0x35

// List of properties for FM mode
#define SI468X_PROP_FM_TUNE_FE_CFG              0x1712
#define SI468X_PROP_FM_RDS_CONFIG               0x3C02
#define SI468X_PROP_FM_AUDIO_DE_EMPHASIS        0x3900

int si468x_fm_tune_freq(uint16_t freq)
{
    uint8_t data_out[7] = {
        SI468X_CMD_FM_TUNE_FREQ,
        0x00,
        (uint8_t)(freq & 0xFF),
        (uint8_t)((freq & 0xFF00) >> 8),
        0,
        0,
        0,
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

int si468x_fm_seek_start(uint8_t seek_up_down, uint8_t wrap)
{
    uint8_t data_out[6] = {
        SI468X_CMD_FM_SEEK_START,
        0x00,    // FORCE_WB = 0; TUNE_MODE = 0; INJECTION = 0
        (seek_up_down & 0x2) | (wrap & 0x1),
        0,
        0,    // ANTCAP = 0 (automatic tuning capacitor)
        0
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

int si468x_fm_rsq_status(struct si468x_FM_rsq_status* rsq_status)
{
    int ret;
    uint8_t data_out[2] = {
        SI468X_CMD_FM_RSQ_STATUS,
        0x80,    // RSQACK=1; ATTUNE=0; CANCEL=0; STCACK=0
    };
    struct si468x_status status;
    uint8_t data_in[17];

    ret = si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status,
                                                    data_in, sizeof(data_in));
    if (ret < 0) {
        ERROR_TRACE();
        return ret;
    }

    rsq_status->snrhint = 0;    //TODO
    rsq_status->snrlint = 0;    //TODO
    rsq_status->rssihint = 0;    //TODO
    rsq_status->rssilint = 0;    //TODO
    rsq_status->bltf = 0;    //TODO
    rsq_status->hddetected = 0;    //TODO
    rsq_status->flt_hd_detected = 0;    //TODO
    rsq_status->afcrl = 0;    //TODO
    rsq_status->valid = 0;    //TODO
    rsq_status->read_freq = (uint16_t)(data_in[3] << 8) + (uint16_t)data_in[2];
    rsq_status->freq_off = (int8_t)data_in[4];
    rsq_status->rssi = (int8_t)data_in[5];
    rsq_status->snr = (int8_t)data_in[6];
    rsq_status->mult = data_in[7];
    rsq_status->readantcap = 0;    //TODO
    rsq_status->hdlevel = 0;    //TODO
    rsq_status->filtered_hdlevel = 0;    //TODO

    return 0;
}

int si468x_fm_rds_status(struct si468x_FM_rds_status* rds_status)
{
    int ret;
    uint8_t data_out[2] = {
        SI468X_CMD_FM_RDS_STATUS,
        0x00,    // STATUSONLY=0; MTFIFO=0; INTACK=0
    };
    struct si468x_status status;
    uint8_t data_in[16];

    ret = si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status,
                                                    data_in, sizeof(data_in));
    if (ret < 0) {
        ERROR_TRACE();
        return ret;
    }

    rds_status->rdstpptyint = !!(data_in[0] & 0x10);
    rds_status->rdspiint = !!(data_in[0] & 0x08);
    rds_status->rdssyncint = !!(data_in[0] & 0x02);
    rds_status->rdsfifoint = !!(data_in[0] & 0x01);
    rds_status->tpptyvalid = !!(data_in[1] & 0x10);
    rds_status->pivalid = !!(data_in[1] & 0x08);
    rds_status->rdssync = !!(data_in[1] & 0x02);
    rds_status->rdsfifolost = !!(data_in[1] & 0x01);
    rds_status->tp = !!(data_in[2] & 0x20);
    rds_status->pty = data_in[2] & 0x1F;
    rds_status->pi = (uint16_t)(data_in[6] << 8) + (uint16_t)(data_in[5]);
    rds_status->rdsfifoused = data_in[6];
    rds_status->blea = (data_in[7] >> 6) & 0x3;
    rds_status->bleb = (data_in[7] >> 4) & 0x3;
    rds_status->blec = (data_in[7] >> 2) & 0x3;
    rds_status->bled = (data_in[7] >> 0) & 0x3;
    rds_status->blocka = (uint16_t)(data_in[9] << 8) + (uint16_t)(data_in[8]);
    rds_status->blockb = (uint16_t)(data_in[11] << 8) + (uint16_t)(data_in[10]);
    rds_status->blockc = (uint16_t)(data_in[13] << 8) + (uint16_t)(data_in[12]);
    rds_status->blockd = (uint16_t)(data_in[15] << 8) + (uint16_t)(data_in[14]);

    return 0;
}

int si468x_fm_rds_blockcount(struct si468x_FM_rds_blockcount* blockcount)
{
    int ret;
    uint8_t data_out[2] = {
        SI468X_CMD_FM_RDS_BLOCKCOUNT,
        0x00,    // CLEAR=0
    };
    struct si468x_status status;
    uint8_t data_in[6];

    ret = si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status,
                                                    data_in, sizeof(data_in));
    if (ret < 0) {
        ERROR_TRACE();
        return ret;
    }

    blockcount->expected = (uint16_t)(data_in[1] << 8) + (uint16_t)(data_in[0]);
    blockcount->received = (uint16_t)(data_in[3] << 8) + (uint16_t)(data_in[2]);
    blockcount->uncorrectable = (uint16_t)(data_in[5] << 8) + (uint16_t)(data_in[4]);

    return 0;
}
