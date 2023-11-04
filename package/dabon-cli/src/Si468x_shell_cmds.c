#include "Si468x.h"
#include "Si468x_ext.h"
#include "Si468x_shell_cmds.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "utils.h"
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

extern uint8_t _binary_external_firmwares_rom00_patch_016_bin_start;
extern uint8_t _binary_external_firmwares_rom00_patch_016_bin_end;
extern uint8_t _binary_external_firmwares_dab_radio_5_0_5_bin_start;
extern uint8_t _binary_external_firmwares_dab_radio_5_0_5_bin_end;
extern uint8_t _binary_external_firmwares_fmhd_radio_5_0_4_bin_start;
extern uint8_t _binary_external_firmwares_fmhd_radio_5_0_4_bin_end;

int stop_tuner();
int quit_program();

int dab_tune_frequency();
int dab_digrad_status();
int dab_get_event_status();
int dab_get_ensamble_info();
int dab_get_digital_service_list();
int dab_start_digital_service();
int dab_quick_start_digital_service();
int dab_get_audio_info();
int dab_get_time();

int dab_scan_frequencies();
int dab_list_european_frequencies();
int dab_monitor_digrad_status();

int fm_tune_frequency();
int fm_seek_start();
int fm_rsq_status();
int fm_rds_status();
int fm_rds_blockcount();

int tuner_get_part_info();
int tuner_get_property();
int tuner_set_property();
int tuner_get_volume();
int tuner_set_volume();

COMMANDS dab_commands[] = {
    {"dab tune frequency", dab_tune_frequency},
    {"dab digrad status", dab_digrad_status},
    {"dab get event status", dab_get_event_status},
    {"dab get ensamble info", dab_get_ensamble_info},
    {"dab get digital service list", dab_get_digital_service_list},
    {"dab start digital service", dab_start_digital_service},
    {"dab quick start digital service", dab_quick_start_digital_service},
    {"dab get audio_info", dab_get_audio_info},
    {"dab get time", dab_get_time},
    {"dab scan frequencies", dab_scan_frequencies},
    {"dab list european frequencies", dab_list_european_frequencies},
    {"dab monitor digrad status", dab_monitor_digrad_status},
    {"get part_info", tuner_get_part_info},
    {"get property", tuner_get_property},
    {"set property", tuner_set_property},
    {"get volume", tuner_get_volume},
    {"set volume", tuner_set_volume},
    {"exit", quit_program},
};

COMMANDS fm_commands[] = {
	{"fm_tune_frequency", fm_tune_frequency},
	{"fm_seek_start", fm_seek_start},
	{"fm_rsq_status", fm_rsq_status},
	{"fm_rds_status", fm_rds_status},
	{"fm_rds_blockcount", fm_rds_blockcount},
    {"get part_info", tuner_get_part_info},
    {"get property", tuner_get_property},
    {"set property", tuner_set_property},
    {"get volume", tuner_get_volume},
    {"set volume", tuner_set_volume},
    {"exit", quit_program},
};

int quit_program()
{
	stop_tuner();
	exit(0);
}

int start_tuner(COMMANDS** tuner_commands, int* tuner_commands_length)
{
	uint8_t* btldr_start;
	uint8_t* btldr_end;
	uint8_t* app_start;
	uint8_t* app_end;
	uint32_t btldr_size, app_size;
	int ret;
	enum tuner_fw tuner_fw_to_load;
	
	printf("Select tuner mode [0=DAB; 1=FM]\n");
	printf("> ");
	if (get_int_from_stdin(0, 1, INPUT_AS_DEC, (int*)&tuner_fw_to_load) < 0) {
		return -1;
	}
	
	btldr_start = &_binary_external_firmwares_rom00_patch_016_bin_start;
	btldr_end = &_binary_external_firmwares_rom00_patch_016_bin_end;
	btldr_size = (uint32_t)(btldr_end - btldr_start);
	
	ret = Si468x_init(); 
    if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}
	
	if (tuner_fw_to_load == DAB_FW) {
		app_start = &_binary_external_firmwares_dab_radio_5_0_5_bin_start;
		app_end = &_binary_external_firmwares_dab_radio_5_0_5_bin_end;
		app_size = (uint32_t)(app_end - app_start);
	} else {
		app_start = &_binary_external_firmwares_fmhd_radio_5_0_4_bin_start;
		app_end = &_binary_external_firmwares_fmhd_radio_5_0_4_bin_end;
		app_size = (uint32_t)(app_end - app_start);
	}
	printf("Loading %s firmware (btldr_start=0x%x - btldr_size=%d - app_start=0x%x - app_size=%d)\n",
					(tuner_fw_to_load == DAB_FW) ? "DAB" : "FM",
					btldr_start, btldr_size, app_start, app_size);
	ret = Si468x_start_image(btldr_start, btldr_size, app_start, app_size);
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}
	
	if (tuner_fw_to_load == DAB_FW) {
		*tuner_commands = dab_commands;
		*tuner_commands_length = ARRAY_SIZE(dab_commands);
	} else {
		*tuner_commands = fm_commands;
		*tuner_commands_length = ARRAY_SIZE(fm_commands);
	}
    
    return 0;
}

int stop_tuner()
{
	int ret;
	
	ret = Si468x_deinit();
    if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}
	
	return 0;
}

int dab_tune_frequency()
{
	int freq_num;
	int max_freq_index = ARRAY_SIZE(bandIII_DAB_blocks_list) - 1;
	
	printf("Select the frequency [0; %d]\n", max_freq_index);
	printf("> ");

	if (get_int_from_stdin(0, max_freq_index, INPUT_AS_DEC, &freq_num) < 0) {
		return -1;
	}
	
	Si468x_dab_tune_freq((uint8_t)freq_num);

	return 0;
}

int dab_digrad_status()
{
	struct Si468x_DAB_digrad_status Si468x_DAB_status;
	
	Si468x_dab_digrad_status(1, 0, 1, &Si468x_DAB_status);

	printf("hardmuteint: %d \n", Si468x_DAB_status.hardmuteint);
	printf("ficerrint: %d \n", Si468x_DAB_status.ficerrint);
	printf("acqint: %d \n", Si468x_DAB_status.acqint);
	printf("rssihint: %d \n", Si468x_DAB_status.rssihint);
	printf("rssilint: %d \n", Si468x_DAB_status.rssilint);
	printf("hardmute: %d \n", Si468x_DAB_status.hardmute);
	printf("ficerr: %d \n", Si468x_DAB_status.ficerr);
	printf("acq: %d \n", Si468x_DAB_status.acq);
	printf("valid: %d \n", Si468x_DAB_status.valid);
	
	printf("RSSI: %d \n", Si468x_DAB_status.rssi);
	printf("SNR: %d \n", Si468x_DAB_status.snr);
	printf("FIC quality: %d \n", Si468x_DAB_status.fic_quality);
	printf("tune frequency: %d \n", Si468x_DAB_status.tune_freq);
	printf("tune index: %d \n", Si468x_DAB_status.tune_index);
	printf("tune offset: %d \n", Si468x_DAB_status.tune_offet);
	printf("fft offset: %d \n", Si468x_DAB_status.fft_offset);
	printf("antenna cap: %d \n", Si468x_DAB_status.readantcap);
	printf("CNR: %d \n", Si468x_DAB_status.cnr);
	printf("FIB error count: %d \n", Si468x_DAB_status.FIB_error_count);
	printf("CU level: %d \n", Si468x_DAB_status.culevel);
	printf("fastdect: %d \n", Si468x_DAB_status.fastdect);
	     
	return 0;
}

int dab_get_event_status()
{
	struct Si468x_DAB_event_status event_status;

	Si468x_dab_get_event_status(Si468x_KEEP_INT, &event_status);
	printf("recfgint: %u\n", event_status.recfgint);
	printf("recfgwrnint: %u\n", event_status.recfgwrnint);
	printf("annoint: %u\n", event_status.annoint);
	printf("freqinfoint: %u\n", event_status.freqinfoint);
	printf("svrlistint: %u\n", event_status.svrlistint);
	printf("freq_info: %u\n", event_status.freq_info);
	printf("svrlist: %u\n", event_status.svrlist);
	printf("svrlistver: %u\n", event_status.svrlistver);
	
	Si468x_dab_get_event_status(Si468x_CLEAR_INT, &event_status);
    
	return 0;
}

int dab_get_ensamble_info()
{
	struct Si468x_DAB_ensamble_info ensamble_info;
	Si468x_dab_get_ensamble_info(&ensamble_info);

	printf("EID: %u\n", ensamble_info.eid);
	printf("Label: %s\n", ensamble_info.label);
	printf("ECC: %u\n", ensamble_info.ensamble_ecc);
	printf("EID: %02X\n", ensamble_info.char_abbrev);

	return 0;
}

int dab_get_digital_service_list()
{
	struct dab_service_list dab_service_list = {0};
	int ret;
	
	ret = Si468x_get_dab_service_list(&dab_service_list);
	if (ret < 0)
		return ret;

	printf("Service list size (bytes): %u\n", dab_service_list.header.service_list_size);
	printf("Service list version: %u\n", dab_service_list.header.service_list_version);
	printf("Number of services: %u\n", dab_service_list.header.num_of_services);
	
	uint8_t service_index, component_index;
	struct Si468x_DAB_digital_service* tmp_service_ptr;
	struct Si468x_DAB_digital_service_component* tmp_component_ptr;
	
	for (service_index = 0; dab_service_list.service_list[service_index] != NULL; service_index++) {
		tmp_service_ptr = dab_service_list.service_list[service_index];
		printf("(%d) Service ID: 0x%08x\n", service_index, tmp_service_ptr->service_id);
		printf("    Pty: %d\n", get_service1_Pty(tmp_service_ptr->service_info_1));
		printf("    P/D flag: %d\n", get_service1_PD_flag(tmp_service_ptr->service_info_1));
		printf("    Service label: %.16s\n", tmp_service_ptr->service_label);

		for (component_index = 0; dab_service_list.components_list[service_index][component_index] != NULL; component_index++) {
			tmp_component_ptr = dab_service_list.components_list[service_index][component_index];
			printf("        (%d) Component ID: 0x%04x\n", component_index, tmp_component_ptr->component_id);
		}
	}
	free(dab_service_list.raw_service_list);
    
	return 0;
}

int dab_start_digital_service()
{
	uint32_t service_id;
	uint32_t component_id;
	int ret;

	printf("Select the service ID [0; 0x7FFFFFFF]\n");
	printf("> ");
	if (get_int_from_stdin(0, 0x7FFFFFFF, INPUT_AS_HEX, &service_id) < 0) {
		return -1;
	}
	printf("Select the component ID [0; 0xFFFF]\n");
	printf("> ");
	if (get_int_from_stdin(0, 0xFFFF, INPUT_AS_HEX, &component_id) < 0) {
		return -1;
	}

	ret = Si468x_start_digital_service(service_id, component_id);
	if (ret < 0)
		return ret;
		
	return 0;
}

int dab_quick_start_digital_service()
{
	uint32_t service_index, services_count;
	int ret, i;
	struct dab_service_list dab_service_list = {0};
	
	ret = Si468x_get_dab_service_list(&dab_service_list);
	if (ret < 0)
		return ret;
	
	services_count = dab_service_list.header.num_of_services;
	for (i=0; i<services_count; i++) {
		printf("(%d) %s\n", i, dab_service_list.service_list[i]->service_label);
	}
	printf("Select the desired service [0; %d]\n", services_count-1);
	printf("> ");
	
	if (get_int_from_stdin(0, services_count-1, INPUT_AS_DEC, &service_index) < 0) {
		return -1;
	}

	ret = Si468x_start_digital_service(dab_service_list.service_list[service_index]->service_id, 
										dab_service_list.components_list[service_index][0]->component_id);
	if (ret < 0)
		return ret;
		
	free(dab_service_list.raw_service_list);
		
	return 0;
}

int dab_get_audio_info()
{
	struct Si468x_DAB_audio_info audio_info;
	Si468x_dab_get_audio_info(&audio_info);

	printf("Bit rate: %u\n", audio_info.bit_rate);
	printf("Sample rate: %u\n", audio_info.sample_rate);
	printf("Audio DRC gain: %u\n", audio_info.drc_gain);
    
	return 0;
}

int dab_get_time()
{
	struct Si468x_DAB_time dab_time;
	int ret;
	
	ret = Si468x_dab_get_time(&dab_time);
	if (ret < 0)
		return ret;
	
	printf("%d:%d:%d - %d/%d/%d\n", 
			dab_time.hours, dab_time.minutes, dab_time.seconds,
			dab_time.day, dab_time.month, dab_time.year);
	
	return 0;
}

int dab_scan_frequencies()
{
	struct Si468x_DAB_freq_list freq_list;
	struct Si468x_DAB_digrad_status Si468x_DAB_status;
	int curr_freq_index;
	int ret;
	
	ret = Si468x_dab_get_freq_list(&freq_list);
	if (ret < 0)
		return ret;
	printf("There are %d frequencies starting at %d\n", freq_list.num_freqs, freq_list.first_freq );
		
	for (curr_freq_index = 0; curr_freq_index < freq_list.num_freqs; curr_freq_index++) {
		ret = Si468x_dab_tune_freq(curr_freq_index);
		if (ret < 0)
			return ret;
		sleep(1);
		
		printf("\nFreq index: %d\n", curr_freq_index);
		ret = dab_digrad_status();
		if (ret < 0)
			return ret;
	}
	
	return 0;
}

int dab_list_european_frequencies()
{
	int i;
	
	for (i = 0; i < ARRAY_SIZE(bandIII_DAB_blocks_list); i++) {
		printf("(%d) %s - %d MHz\n", i, bandIII_DAB_blocks_list[i].label, bandIII_DAB_blocks_list[i].frequency_MHz);
	}
	
	return 0;
}

int dab_monitor_digrad_status()
{
	struct Si468x_DAB_digrad_status Si468x_DAB_status;
	WINDOW *w;
    char c = 0;
    time_t t;
	
	w = initscr();
	
	do {
		Si468x_dab_digrad_status(1, 0, 1, &Si468x_DAB_status);
		
		clear();
		time(&t);
		printw("=== %s", ctime(&t));
		printw("hardmuteint: %d \n", Si468x_DAB_status.hardmuteint);
		printw("ficerrint: %d \n", Si468x_DAB_status.ficerrint);
		printw("acqint: %d \n", Si468x_DAB_status.acqint);
		printw("rssihint: %d \n", Si468x_DAB_status.rssihint);
		printw("rssilint: %d \n", Si468x_DAB_status.rssilint);
		printw("hardmute: %d \n", Si468x_DAB_status.hardmute);
		printw("ficerr: %d \n", Si468x_DAB_status.ficerr);
		printw("acq: %d \n", Si468x_DAB_status.acq);
		printw("valid: %d \n", Si468x_DAB_status.valid);
		printw("RSSI: %d \n", Si468x_DAB_status.rssi);
		printw("SNR: %d \n", Si468x_DAB_status.snr);
		printw("FIC quality: %d \n", Si468x_DAB_status.fic_quality);
		printw("tune frequency: %d \n", Si468x_DAB_status.tune_freq);
		printw("tune index: %d \n", Si468x_DAB_status.tune_index);
		printw("tune offset: %d \n", Si468x_DAB_status.tune_offet);
		printw("fft offset: %d \n", Si468x_DAB_status.fft_offset);
		printw("antenna cap: %d \n", Si468x_DAB_status.readantcap);
		printw("CNR: %d \n", Si468x_DAB_status.cnr);
		printw("FIB error count: %d \n", Si468x_DAB_status.FIB_error_count);
		printw("CU level: %d \n", Si468x_DAB_status.culevel);
		printw("fastdect: %d \n", Si468x_DAB_status.fastdect);
		printw("(Press space bar to terminate the scan)\n");
		refresh();
		
		timeout(1000);
		c = getch();
	} while  (c != ' ');
	
	endwin();
	
	return 0;
}

int fm_tune_frequency()
{
	int freq;
	int ret;
	
	printf("Select the frequency in kHz [87500; 108000]\n", freq);
	printf("> ");

	if (get_int_from_stdin(87500, 108000, INPUT_AS_DEC, &freq) < 0) {
		return -1;
	}
	
	ret = Si468x_fm_tune_freq((uint16_t)(freq/10));
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int fm_seek_start()
{
	int ret;
	int up_down;
	
	printf("Which direction? [0=down; 1=up]\n", &up_down);
	printf("> ");
	
	if (get_int_from_stdin(0, 1, INPUT_AS_DEC, &up_down) < 0) {
		return -1;
	}
	
	ret = Si468x_fm_seek_start((up_down) ? FM_SEEK_UP : FM_SEEK_DOWN, FM_SEEK_WRAP);
	if (ret < 0) {
		return ret;
	}
	
	return 0;
}

int fm_rsq_status()
{
	int ret;
	struct Si468x_FM_rsq_status status;
	
	ret = Si468x_fm_rsq_status(&status);
	if (ret < 0) {
		return ret;
	}
	
	printf("read_freq: %u\n", status.read_freq*10);
	printf("rssi: %d dBuV\n", status.rssi);
	printf("snr: %d dB\n", status.snr);
	
	return 0;
}

int fm_rds_status()
{
	int ret;
	struct Si468x_FM_rds_status status;
	
	ret = Si468x_fm_rds_status(&status);
	if (ret < 0) {
		return ret;
	}
	
	dump_array((uint8_t*)&status, sizeof(struct Si468x_FM_rds_status));
	
	printf("pty: %u\n", status.pty);
	printf("pi: %u\n", status.pi);
	printf("rdsfifoused: %u\n", status.rdsfifoused);
	printf("blocka: %u\n", status.blocka);
	printf("blockb: %u\n", status.blockb);
	printf("blockc: %u\n", status.blockc);
	printf("blockd: %u\n", status.blockd);
	
	return 0;
}

int fm_rds_blockcount()
{
	int ret;
	struct Si468x_FM_rds_blockcount blockcount;
	
	ret = Si468x_fm_rds_blockcount(&blockcount);
	if (ret < 0) {
		return ret;
	}
	
	printf("expected: %u\n", blockcount.expected);
	printf("received: %u\n", blockcount.received);
	printf("uncorrectable: %u\n", blockcount.uncorrectable);
	
	return 0;
}

int tuner_get_part_info()
{
	int ret;
	struct Si468x_info info;
	
	ret = Si468x_get_part_info(&info);
	if (ret < 0) {
		return ret;
	}
	
	printf("chiprev: %u\n", info.chiprev);
	printf("romid: %u\n", info.romid);
	printf("part: %u\n", info.part);
	
	return 0;
}

int tuner_get_property()
{
	int property_index, ret;
	uint16_t property_val;
	
	printf("Select the property [0x0000; 0xE800]\n");
	printf("> ");
	if (get_int_from_stdin(0, 0xe800, INPUT_AS_HEX, &property_index) < 0) {
		return -1;
	}
	
	ret = Si468x_get_property(property_index, &property_val);
	if (ret < 0)
		return ret;
		
	printf("Value --> (dec)%d - (hex)0x%x\n", property_val, property_val);
	
	return 0;
}

int tuner_set_property()
{
	int property_index, ret;
	int property_val;
	
	printf("Select the property [0x0000; 0xE800]\n");
	printf("> ");
	if (get_int_from_stdin(0, 0xe800, INPUT_AS_HEX, &property_index) < 0) {
		return -1;
	}
	
	printf("Select the value [0x0000; 0xFFFF]\n");
	printf("> ");
	if (get_int_from_stdin(0, 0xe800, INPUT_AS_HEX, &property_val) < 0) {
		return -1;
	}
	
	ret = Si468x_set_property(property_index, (uint16_t)property_val);
	if (ret < 0)
		return ret;
	
	return 0;
}

int tuner_get_volume()
{
	uint16_t property_val;
	int ret;
	
	ret = Si468x_get_property(SI468X_PROP_AUDIO_ANALOG_VOLUME, &property_val);
	if (ret < 0)
		return ret;
	
	printf("Volume: %d\n", property_val);
	
	return 0;
}

int tuner_set_volume()
{
	int property_val;
	int ret;
	
	printf("Set volume value [0; 100]\n");
	printf("> ");
	if (get_int_from_stdin(0, 100, INPUT_AS_DEC, &property_val) < 0) {
		return -1;
	}
	
	ret = Si468x_set_property(SI468X_PROP_AUDIO_ANALOG_VOLUME, (uint16_t)property_val);
	if (ret < 0)
		return ret;
	
	return 0;
}
