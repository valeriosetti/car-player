#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "shell_cmds.h"
#include "si468x.h"
#include "si468x_fm.h"
#include "si468x_dab.h"
#include "si468x_ext.h"
#include "si468x_platform.h"
#include "si468x_utils.h"

int start_tuner(int argc, char *argv[])
{
    (void) argc;
    uint8_t *btldr = NULL;
    uint8_t *app = NULL;
    size_t btldr_size, app_size;
    int ret;

    ret = read_file_to_buffer(argv[1], &btldr, &btldr_size);
    if (ret != 0) {
        ERROR("read error on %s", argv[1]);
        goto exit;
    }

    ret = read_file_to_buffer(argv[2], &app, &app_size);
    if (ret != 0) {
        ERROR("read error on %s", argv[2]);
        goto exit;
    }

    ret = si468x_start_image(btldr, (uint32_t) btldr_size, app, (uint32_t) app_size);
    if (ret < 0) {
        ERROR("FW loading failed");
        goto exit;
    }

exit:
    if (btldr)
        free(btldr);
    if (app)
        free(app);

    return ret;
}

int stop_tuner(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    int ret;
    ret = si468x_gpio_assert_reset();
    if (ret < 0) {
        ERROR("deinitialization failed");
        return ret;
    }
    return 0;
}

int dab_tune_frequency(int argc, char *argv[])
{
    (void) argc;
    int freq_num;

    if (sscanf(argv[1], "%d", &freq_num) != 1) {
        ERROR("Wrong input value %s", argv[1]);
        return -1;
    }

    si468x_dab_tune_freq((uint8_t)freq_num);

    return 0;
}

int dab_digrad_status(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    struct si468x_DAB_digrad_status si468x_DAB_status;

    si468x_dab_digrad_status(1, 0, 1, &si468x_DAB_status);

    PRINT("hardmuteint: %d ", si468x_DAB_status.hardmuteint);
    PRINT("ficerrint: %d ", si468x_DAB_status.ficerrint);
    PRINT("acqint: %d ", si468x_DAB_status.acqint);
    PRINT("rssihint: %d ", si468x_DAB_status.rssihint);
    PRINT("rssilint: %d ", si468x_DAB_status.rssilint);
    PRINT("hardmute: %d ", si468x_DAB_status.hardmute);
    PRINT("ficerr: %d ", si468x_DAB_status.ficerr);
    PRINT("acq: %d ", si468x_DAB_status.acq);
    PRINT("valid: %d ", si468x_DAB_status.valid);
    PRINT("RSSI: %d ", si468x_DAB_status.rssi);
    PRINT("SNR: %d ", si468x_DAB_status.snr);
    PRINT("FIC quality: %d ", si468x_DAB_status.fic_quality);
    PRINT("tune frequency: %d ", si468x_DAB_status.tune_freq);
    PRINT("tune index: %d ", si468x_DAB_status.tune_index);
    PRINT("tune offset: %d ", si468x_DAB_status.tune_offet);
    PRINT("fft offset: %d ", si468x_DAB_status.fft_offset);
    PRINT("antenna cap: %d ", si468x_DAB_status.readantcap);
    PRINT("CNR: %d ", si468x_DAB_status.cnr);
    PRINT("FIB error count: %d ", si468x_DAB_status.FIB_error_count);
    PRINT("CU level: %d ", si468x_DAB_status.culevel);
    PRINT("fastdect: %d ", si468x_DAB_status.fastdect);

    return 0;
}

int dab_get_event_status(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    struct si468x_DAB_event_status event_status;

    si468x_dab_get_event_status(si468x_KEEP_INT, &event_status);

    PRINT("recfgint: %u", event_status.recfgint);
    PRINT("recfgwrnint: %u", event_status.recfgwrnint);
    PRINT("annoint: %u", event_status.annoint);
    PRINT("freqinfoint: %u", event_status.freqinfoint);
    PRINT("svrlistint: %u", event_status.svrlistint);
    PRINT("freq_info: %u", event_status.freq_info);
    PRINT("svrlist: %u", event_status.svrlist);
    PRINT("svrlistver: %u", event_status.svrlistver);

    si468x_dab_get_event_status(si468x_CLEAR_INT, &event_status);

    return 0;
}

int dab_get_ensamble_info(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    struct si468x_DAB_ensamble_info ensamble_info;
    si468x_dab_get_ensamble_info(&ensamble_info);

    PRINT("EID: %u", ensamble_info.eid);
    PRINT("Label: %s", ensamble_info.label);
    PRINT("ECC: %u", ensamble_info.ensamble_ecc);
    PRINT("EID: %02X", ensamble_info.char_abbrev);

    return 0;
}

int dab_get_digital_service_list(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    struct dab_service_list dab_service_list = {0};
    int ret;
    ret = si468x_dab_get_service_list(&dab_service_list);
    if (ret < 0)
        return ret;

    PRINT("Service list size (bytes): %u", dab_service_list.header.service_list_size);
    PRINT("Service list version: %u", dab_service_list.header.service_list_version);
    PRINT("Number of services: %u", dab_service_list.header.num_of_services);

    uint8_t service_index, component_index;
    struct si468x_DAB_digital_service* tmp_service_ptr;
    struct si468x_DAB_digital_service_component* tmp_component_ptr;
    for (service_index = 0; dab_service_list.service_list[service_index] != NULL; service_index++) {
        tmp_service_ptr = dab_service_list.service_list[service_index];
        PRINT("(%d) Service ID: 0x%08x", service_index, tmp_service_ptr->service_id);
        PRINT("    Pty: %d", get_service1_Pty(tmp_service_ptr->service_info_1));
        PRINT("    P/D flag: %d", get_service1_PD_flag(tmp_service_ptr->service_info_1));
        PRINT("    Service label: %.16s", tmp_service_ptr->service_label);

        for (component_index = 0; dab_service_list.components_list[service_index][component_index] != NULL; component_index++) {
            tmp_component_ptr = dab_service_list.components_list[service_index][component_index];
            PRINT("        (%d) Component ID: 0x%04x", component_index, tmp_component_ptr->component_id);
        }
    }
    free(dab_service_list.raw_service_list);
    return 0;
}

int dab_start_digital_service(int argc, char *argv[])
{
    (void) argc;
    uint32_t service_id;
    uint32_t component_id;
    int ret;

    if (sscanf(argv[1], "%d", &service_id) != 1) {
        ERROR("Invalid service ID %s", argv[1]);
        return -1;
    }

    if (sscanf(argv[2], "%d", &component_id) != 1) {
        ERROR("Invalid component ID %s", argv[2]);
        return -1;
    }

    ret = si468x_dab_start_digital_service(service_id, component_id);
    if (ret < 0)
        return ret;

    return 0;
}

int dab_get_audio_info(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    struct si468x_DAB_audio_info audio_info;
    si468x_dab_get_audio_info(&audio_info);

    PRINT("Bit rate: %u", audio_info.bit_rate);
    PRINT("Sample rate: %u", audio_info.sample_rate);
    PRINT("Audio DRC gain: %u", audio_info.drc_gain);

    return 0;
}

int dab_get_time(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    struct si468x_DAB_time dab_time;
    int ret;
    ret = si468x_dab_get_time(&dab_time);
    if (ret < 0)
        return ret;

    PRINT("%d:%d:%d - %d/%d/%d", 
            dab_time.hours, dab_time.minutes, dab_time.seconds,
            dab_time.day, dab_time.month, dab_time.year);

    return 0;
}

int dab_get_frequencies_list(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    uint32_t *frequencies;
    size_t freq_count;
    int ret;

    ret = si468x_dab_get_freq_list(&frequencies, &freq_count);
    if (ret < 0)
        return ret;

    for (size_t i = 0; i < freq_count; i++) {
        PRINT("(%lu) %d kHz", i, frequencies[i]);
    }

    free(frequencies);

    return 0;
}

int dab_set_frequencies_list(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    return si468x_dab_set_freq_list();
}

int fm_tune_frequency(int argc, char *argv[])
{
    (void) argc;
    int freq;
    int ret;

    if (sscanf(argv[1], "%d", &freq) != 1) {
        ERROR("Unable to parse frequency value %s", argv[1]);
        return -1;
    }
    if ((freq < 87500) || (freq > 108000)) {
        ERROR("Invalid frequency value %d", freq);
        return -1;
    }

    ret = si468x_fm_tune_freq((uint16_t)(freq/10));
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int fm_seek_start(int argc, char *argv[])
{
    (void) argc;
    int ret;
    int up_down;

    if (sscanf(argv[1], "%d", &up_down) != 1) {
        ERROR("Unable to parse up/down value %s", argv[1]);
        return -1;
    }
    if ((up_down != 0) && (up_down != 1)) {
        ERROR("Invalid frequency value %d", up_down);
        return -1;
    }

    ret = si468x_fm_seek_start((up_down) ? FM_SEEK_UP : FM_SEEK_DOWN, FM_SEEK_WRAP);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int fm_rsq_status(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    int ret;
    struct si468x_FM_rsq_status status;

    ret = si468x_fm_rsq_status(&status);
    if (ret < 0) {
        return ret;
    }

    PRINT("read_freq: %u", status.read_freq*10);
    PRINT("rssi: %d dBuV", status.rssi);
    PRINT("snr: %d dB", status.snr);

    return 0;
}

int fm_rds_status(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    int ret;
    struct si468x_FM_rds_status status;

    ret = si468x_fm_rds_status(&status);
    if (ret < 0) {
        return ret;
    }

    PRINT("pty: %u", status.pty);
    PRINT("pi: %u", status.pi);
    PRINT("rdsfifoused: %u", status.rdsfifoused);
    PRINT("blocka: %u", status.blocka);
    PRINT("blockb: %u", status.blockb);
    PRINT("blockc: %u", status.blockc);
    PRINT("blockd: %u", status.blockd);

    return 0;
}

int fm_rds_blockcount(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    int ret;
    struct si468x_FM_rds_blockcount blockcount;

    ret = si468x_fm_rds_blockcount(&blockcount);
    if (ret < 0) {
        return ret;
    }

    PRINT("expected: %u", blockcount.expected);
    PRINT("received: %u", blockcount.received);
    PRINT("uncorrectable: %u", blockcount.uncorrectable);

    return 0;
}

int tuner_get_part_info(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    int ret;
    struct si468x_info info;

    ret = si468x_get_part_info(&info);
    if (ret < 0) {
        return ret;
    }

    PRINT("chiprev: %u", info.chiprev);
    PRINT("romid: %u", info.romid);
    PRINT("part: %u", info.part);

    return 0;
}

int tuner_get_property(int argc, char *argv[])
{
    (void) argc;
    int property_index, ret;
    uint16_t property_val;

    if (sscanf(argv[1], "%x", &property_index) != 1) {
        ERROR("Unable to parse property index %s", argv[1]);
        return -1;
    }
    if ((property_index < 0) || (property_index > 0xe800)) {
        ERROR("Invalid property index %d", property_index);
        return -1;
    }

    ret = si468x_get_property(property_index, &property_val);
    if (ret < 0)
        return ret;
    PRINT("%d (0x%x)", property_val, property_val);

    return 0;
}

int tuner_set_property(int argc, char *argv[])
{
    (void) argc;
    int property_index, ret;
    int property_val;

    if (sscanf(argv[1], "%x", &property_index) != 1) {
        ERROR("Unable to parse property index %s", argv[1]);
        return -1;
    }
    if ((property_index < 0) || (property_index > 0xe800)) {
        ERROR("Invalid property index %d", property_index);
        return -1;
    }

    if (sscanf(argv[2], "%x", &property_val) != 1) {
        ERROR("Unable to parse property value %s", argv[2]);
        return -1;
    }

    ret = si468x_set_property(property_index, (uint16_t)property_val);
    if (ret < 0)
        return ret;

    return 0;
}

SHELL_COMMAND shell_commands[] = {
    /* Common */
    {"start_tuner", start_tuner},
    {"stop_tuner", stop_tuner},
    {"get_property", tuner_get_property},
    {"set_property", tuner_set_property},
    {"get_part_info", tuner_get_part_info},
    /* DAB */
    {"dab_tune_frequency", dab_tune_frequency},
    {"dab_digrad_status", dab_digrad_status},
    {"dab_get_event_status", dab_get_event_status},
    {"dab_get_ensamble_info", dab_get_ensamble_info},
    {"dab_get_digital_service_list", dab_get_digital_service_list},
    {"dab_start_digital_service", dab_start_digital_service},
    {"dab_get_audio_info", dab_get_audio_info},
    {"dab_get_time", dab_get_time},
    {"dab_get_frequencies_list", dab_get_frequencies_list},
    {"dab_set_frequencies_list", dab_set_frequencies_list},
    /* FM */
    {"fm_tune_frequency", fm_tune_frequency},
    {"fm_seek_start", fm_seek_start},
    {"fm_rsq_status", fm_rsq_status},
    {"fm_rds_status", fm_rds_status},
    {"fm_rds_blockcount", fm_rds_blockcount},
};

int shell_commands_count = ARRAY_SIZE(shell_commands);
