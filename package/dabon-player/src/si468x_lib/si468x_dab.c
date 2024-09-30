#include <stdlib.h>
#include <string.h>

#include "si468x.h"
#include "si468x_dab.h"
#include "si468x_int.h"
#include "si468x_platform.h"

// List of commands
#define SI468X_CMD_GET_DIGITAL_SERVICE_LIST                 0x80
#define SI468X_CMD_START_DIGITAL_SERVICE                    0x81
#define SI468X_CMD_STOP_DIGITAL_SERVICE                     0x82
#define SI468X_CMD_GET_DIGITAL_SERVICE_DATA                 0x84
#define SI468X_CMD_DAB_TUNE_FREQ                            0xB0
#define SI468X_CMD_DAB_DIGRAD_STATUS                        0xB2
#define SI468X_CMD_DAB_GET_EVENT_STATUS                     0xB3
#define SI468X_CMD_DAB_GET_ENSEMBLE_INFO                    0xB4
#define SI468X_CMD_DAB_GET_SERVICE_LINKING_INFO             0xB7
#define SI468X_CMD_DAB_SET_FREQ_LIST                        0xB8
#define SI468X_CMD_DAB_GET_FREQ_LIST                        0xB9
#define SI468X_CMD_DAB_GET_COMPONENT_INFO                   0xBB
#define SI468X_CMD_DAB_GET_TIME                             0xBC
#define SI468X_CMD_DAB_GET_AUDIO_INFO                       0xBD
#define SI468X_CMD_DAB_GET_SUBCHAN_INFO                     0xBE
#define SI468X_CMD_DAB_GET_FREQ_INFO                        0xBF
#define SI468X_CMD_DAB_GET_SERVICE_INFO                     0xC0
#define SI468X_CMD_TEST_GET_RSSI                            0xE5
#define SI468X_CMD_DAB_TEST_GET_BER_INFO                    0xE8

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

int si468x_dab_get_freq_list(uint32_t **list, size_t *items_count)
{
    uint8_t data_out[2] = {
        SI468X_CMD_DAB_GET_FREQ_LIST,
        0x00
    };
    si468x_send_command(data_out, sizeof(data_out));

    struct si468x_status status;
    int ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    /* Get the length of the data to read */
    uint8_t frequencies_count;
    si468x_read_reply(&frequencies_count, sizeof(frequencies_count), &status);
    *items_count = frequencies_count;

    int output_list_size = frequencies_count * sizeof(uint32_t);
    *list = malloc(output_list_size);
    if (*list == NULL) {
        ERROR_TRACE();
        return -1;
    }

    /* The extra 4 bytes at the beginning is because we need to re-read all the output. */
    uint8_t *data_in = malloc(4 + output_list_size);
    if (data_in == NULL) {
        free(*list);
        ERROR_TRACE();
        return -1;
    }
    si468x_read_reply(data_in, 4 + output_list_size, &status);

    int i, offset = 4;
    for (i = 0; i < frequencies_count; i++, offset += 4) {
        (*list)[i] = (((uint32_t) data_in[offset + 0]) << 0) +
                     (((uint32_t) data_in[offset + 1]) << 8) +
                     (((uint32_t) data_in[offset + 2]) << 16) +
                     (((uint32_t) data_in[offset + 3]) << 24);
    }

    free(data_in);

    return 0;
}

int si468x_dab_set_freq_list(void)
{
    uint8_t data_out[4 + 4*ARRAY_SIZE(bandIII_DAB_frequency_list)] = {
        SI468X_CMD_DAB_SET_FREQ_LIST,
        ARRAY_SIZE(bandIII_DAB_frequency_list),
        0,
        0
    };

    int offset = 4;
    for (size_t i = 0; i < ARRAY_SIZE(bandIII_DAB_frequency_list); i++) {
        data_out[offset++] = BYTE_0(bandIII_DAB_frequency_list[i].frequency_kHz);
        data_out[offset++] = BYTE_1(bandIII_DAB_frequency_list[i].frequency_kHz);
        data_out[offset++] = BYTE_2(bandIII_DAB_frequency_list[i].frequency_kHz);
        data_out[offset++] = BYTE_3(bandIII_DAB_frequency_list[i].frequency_kHz);
    }

    si468x_send_command(data_out, sizeof(data_out));

    struct si468x_status status;
    int ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }
    return 0;
}

int si468x_dab_tune_freq(uint8_t freq)
{
    uint8_t data_out[6] = {
        SI468X_CMD_DAB_TUNE_FREQ,
        0, // INJECTION = 0 (automatic)
        freq,
        0x00,
        0, // ANTCAP = 0 (automatic)
        0,
    };
    si468x_send_command(data_out, sizeof(data_out));

    struct si468x_status status;
    int ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_STCINT);
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    return 0;
}

int si468x_dab_digrad_status(uint8_t digrad_ack, uint8_t attune, uint8_t stc_ack,
                            struct si468x_DAB_digrad_status *digrad_status)
{
    (void) attune;
    uint8_t data_out[2] = {
        SI468X_CMD_DAB_DIGRAD_STATUS,
        (0x01 & stc_ack) | ((digrad_ack) ? 0x08 : 0x04),
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

    digrad_status->hardmuteint = !!(data_in[0] & 0x10);
    digrad_status->ficerrint = !!(data_in[0] & 0x08);
    digrad_status->acqint = !!(data_in[0] & 0x04);
    digrad_status->rssihint = !!(data_in[0] & 0x02);
    digrad_status->rssilint = !!(data_in[0] & 0x01);

    digrad_status->hardmute = !!(data_in[1] & 0x10);
    digrad_status->ficerr = !!(data_in[1] & 0x08);
    digrad_status->acq = !!(data_in[1] & 0x04);
    digrad_status->valid = !!(data_in[1] & 0x01);

    digrad_status->rssi             = (int8_t)data_in[2];
    digrad_status->snr                 = data_in[3];
    digrad_status->fic_quality         = data_in[4];
    digrad_status->cnr                = data_in[5];
    digrad_status->FIB_error_count = data_in[6] | (uint16_t)(data_in[7] << 8);
    digrad_status->tune_freq        =     (uint32_t)data_in[8] |
                                (uint32_t)(data_in[9] << 8) |
                                (uint32_t)(data_in[10] << 16)|
                                (uint32_t)(data_in[11] << 24);
    digrad_status->tune_index        = data_in[12];
    digrad_status->fft_offset        = (int8_t)data_in[13];
    digrad_status->readantcap        = (uint16_t)data_in[14] | (uint16_t)(data_in[15] << 8);
    digrad_status->culevel            = (uint16_t)data_in[16] | (uint16_t)(data_in[17] << 8);
    digrad_status->fastdect            = data_in[18];

    return 0;
}

int si468x_dab_get_ensamble_info(struct si468x_DAB_ensamble_info* ensamble_info)
{
    uint8_t data_out[2] = {
        SI468X_CMD_DAB_GET_ENSEMBLE_INFO,
        0x00
    };
    si468x_send_command(data_out, sizeof(data_out));

    struct si468x_status status;
    int ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    uint8_t data_in[22];
    si468x_read_reply(data_in, sizeof(data_in), &status);

    ensamble_info->eid = (uint16_t)((data_in[1] << 8) | data_in[0]);

    uint8_t i;
    for(i = 0; i < 16; i++) {
        ensamble_info->label[i] = data_in[2 + i];
    }
    ensamble_info->label[17] = '\0';
    ensamble_info->ensamble_ecc = data_in[18];
    ensamble_info->char_abbrev = (uint16_t)((data_in[21] << 8) | data_in[20]);

    return 0;
}

int si468x_dab_get_time(struct si468x_DAB_time* dab_time)
{
    uint8_t data_out[2] = {
        SI468X_CMD_DAB_GET_TIME,
        0x00
    };
    si468x_send_command(data_out, sizeof(data_out));

    struct si468x_status status;
    int ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    uint8_t data_in[7];
    si468x_read_reply(data_in, sizeof(data_in), &status);

    dab_time->year = ((uint16_t)(data_in[1] << 8) | data_in[0]);
    dab_time->month = data_in[2];
    dab_time->day = data_in[3];
    dab_time->hours = data_in[4];
    dab_time->minutes = data_in[5];
    dab_time->seconds = data_in[6];

    return 0;
}

#define DIGITAL_SERVICE_LIST_BUFFER_SIZE        2049
int si468x_dab_get_digital_service_list(struct si468x_DAB_digital_service_list_header* list_header,
                                        uint8_t** raw_service_list)
{
    uint8_t data_out[2] = {
        SI468X_CMD_GET_DIGITAL_SERVICE_LIST,
        0x00
    };
    si468x_send_command(data_out, sizeof(data_out));

    struct si468x_status status;
    int ret_val = si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
    if (ret_val < 0) {
        ERROR_TRACE();
        return ret_val;
    }

    uint8_t* tmp_buffer = (uint8_t*) malloc(DIGITAL_SERVICE_LIST_BUFFER_SIZE);
    if (tmp_buffer == NULL) {
        ERROR_TRACE();
        return ret_val;
    }
    si468x_read_reply(tmp_buffer, DIGITAL_SERVICE_LIST_BUFFER_SIZE, &status);

    list_header->service_list_size = (uint16_t)((tmp_buffer[1] << 8) | tmp_buffer[0]);
    list_header->service_list_version = (uint16_t)((tmp_buffer[3] << 8) | tmp_buffer[2]);
    list_header->num_of_services = tmp_buffer[4];

    *raw_service_list = (uint8_t*) malloc(list_header->service_list_size);
    if (*raw_service_list == NULL) {
        ERROR_TRACE();
        free(tmp_buffer);
        return ret_val;
    }
    memcpy(*raw_service_list, &tmp_buffer[8], list_header->service_list_size);
    free(tmp_buffer);

    return 0;
}

int si468x_dab_start_digital_service(uint32_t service_id, uint32_t component_id)
{
    uint8_t data_out[12] = {
        SI468X_CMD_START_DIGITAL_SERVICE,
        0x00,
        0x00,
        0x00,
        (uint8_t)(service_id & 0x000000FF),
        (uint8_t)((service_id & 0x0000FF00) >> 8),
        (uint8_t)((service_id & 0x00FF0000) >> 16),
        (uint8_t)((service_id & 0xFF000000) >> 24),
        (uint8_t)(component_id & 0x000000FF),
        (uint8_t)((component_id & 0x0000FF00) >> 8),
        (uint8_t)((component_id & 0x00FF0000) >> 16),
        (uint8_t)((component_id & 0xFF000000) >> 24),
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

int si468x_dab_get_event_status(uint8_t event_ack, struct si468x_DAB_event_status* event_status)
{
    uint8_t data_out[2] = {
        SI468X_CMD_DAB_GET_EVENT_STATUS,
        (event_ack & 0x01)
    };
    int ret; 
    struct si468x_status status;
    uint8_t data_in[4];

    ret = si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status,
                                                    data_in, sizeof(data_in));
    if (ret < 0) {
        ERROR_TRACE();
        return ret;
    }

    event_status->recfgint = !!(data_in[0] & 0x80);
    event_status->recfgwrnint = !!(data_in[0] & 0x40);
    event_status->annoint = !!(data_in[0] & 0x08);
    event_status->freqinfoint = !!(data_in[0] & 0x02);
    event_status->svrlistint = !!(data_in[0] & 0x01);
    event_status->freq_info = !!(data_in[1] & 0x02);
    event_status->svrlist = !!(data_in[1] & 0x01);
    event_status->svrlistver = ((uint16_t)data_in[3] << 8) | (uint16_t)data_in[2];

    return 0;
}

int si468x_dab_get_audio_info(struct si468x_DAB_audio_info* audio_info)
{
    uint8_t data_out[2] = {
        SI468X_CMD_DAB_GET_AUDIO_INFO,
        0x01
    };
    int ret; 
    struct si468x_status status;
    uint8_t data_in[6];

    ret = si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status,
                                                    data_in, sizeof(data_in));
    if (ret < 0) {
        ERROR_TRACE();
        return ret;
    }

    audio_info->bit_rate = ((uint16_t)(data_in[1] << 8) | (uint16_t)(data_in[0]));
    audio_info->sample_rate = ((uint16_t)(data_in[3] << 8) | (uint16_t)(data_in[2]));
    audio_info->drc_gain = data_in[5];

    switch (data_in[4] & 0x03) {
        case 0:
            audio_info->mode = DAB_AUDIO_MODE_DUAL;
            break;
        case 1:
            audio_info->mode = DAB_AUDIO_MODE_MONO;
            break;
        case 2:
            audio_info->mode = DAB_AUDIO_MODE_STEREO;
            break;
        case 3:
            audio_info->mode = DAB_AUDIO_MODE_JOIN_STEREO;
            break;
    }

    return 0;
}
