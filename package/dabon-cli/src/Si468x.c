#include "Si468x.h"
#include "Si468x_platform.h"
#include "string.h"
#include "stdlib.h"
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define REPLACE_INTERRUPT_WITH_POLLING

// low level timings
#define WAIT_FOR_STATUS_BIT_POLLING_PERIOD_US		1000
#define WAIT_FOR_STATUS_BIT_MAX_TIMEOUT_MS			2000

#define LOAD_BOOTLOADER_IMAGE		0x00
#define LOAD_FM_IMAGE				0x01
#define LOAD_DAB_IMAGE				0x02

// List of commands for DAB mode
#define SI468X_CMD_RD_REPLY								0x00
#define SI468X_CMD_POWER_UP						    		0x01
#define SI468X_CMD_HOST_LOAD					    			0x04
#define SI468X_CMD_FLASH_LOAD					    		0x05
#define SI468X_CMD_LOAD_INIT					    			0x06
#define SI468X_CMD_BOOT							    		0x07
#define SI468X_CMD_GET_PART_INFO							0x08
#define SI468X_CMD_GET_SYS_STATE				    			0x09
#define SI468X_CMD_GET_POWER_UP_ARGS                		0x0A
#define SI468X_CMD_READ_OFFSET                          	0x10
#define SI468X_CMD_GET_FUNC_INFO                        	0x12
#define SI468X_CMD_SET_PROPERTY                         	0x13
#define SI468X_CMD_GET_PROPERTY                         	0x14
#define SI468X_CMD_WRITE_STORAGE                        	0x15
#define SI468X_CMD_READ_STORAGE                         	0x16
#define SI468X_CMD_GET_DIGITAL_SERVICE_LIST             	0x80
#define SI468X_CMD_START_DIGITAL_SERVICE                	0x81
#define SI468X_CMD_STOP_DIGITAL_SERVICE                 	0x82
#define SI468X_CMD_GET_DIGITAL_SERVICE_DATA             	0x84
#define SI468X_CMD_DAB_TUNE_FREQ                        	0xB0
#define SI468X_CMD_DAB_DIGRAD_STATUS                    	0xB2
#define SI468X_CMD_DAB_GET_EVENT_STATUS                 	0xB3
#define SI468X_CMD_DAB_GET_ENSEMBLE_INFO                	0xB4
#define SI468X_CMD_DAB_GET_SERVICE_LINKING_INFO         	0xB7
#define SI468X_CMD_DAB_SET_FREQ_LIST                    	0xB8
#define SI468X_CMD_DAB_GET_FREQ_LIST                    	0xB9
#define SI468X_CMD_DAB_GET_COMPONENT_INFO               	0xBB
#define SI468X_CMD_DAB_GET_TIME                         	0xBC
#define SI468X_CMD_DAB_GET_AUDIO_INFO                   	0xBD
#define SI468X_CMD_DAB_GET_SUBCHAN_INFO                 	0xBE
#define SI468X_CMD_DAB_GET_FREQ_INFO                    	0xBF
#define SI468X_CMD_DAB_GET_SERVICE_INFO                 	0xC0
#define SI468X_CMD_TEST_GET_RSSI                        	0xE5
#define SI468X_CMD_DAB_TEST_GET_BER_INFO                	0xE8

// List of command for FM mode
#define SI468X_CMD_FM_TUNE_FREQ							0x30
#define SI468X_CMD_FM_SEEK_START						0x31
#define SI468X_CMD_FM_RSQ_STATUS						0x32
#define SI468X_CMD_FM_RDS_STATUS						0x34
#define SI468X_CMD_FM_RDS_BLOCKCOUNT					0x35

// Minimum and maximum values for VARB and VARM
#define SI468X_VARB_MIN				-32767
#define SI468X_VARB_MAX				32768
#define SI468X_VARM_MIN				-32767
#define SI468X_VARM_MAX				32768

// List of properties for FM mode
#define SI468X_PROP_FM_TUNE_FE_CFG					0x1712
#define SI468X_PROP_FM_RDS_CONFIG					0x3C02
#define SI468X_PROP_FM_AUDIO_DE_EMPHASIS			0x3900

// Properties
#define SI468X_XTAL_FREQ						19200000L
#define SI468X_XTAL_CL							10	// in pF - that's taken from the crystal's datasheet!
#define SI468X_XTAL_STARTUP_BIAS_CURRENT		800	// in uA
												// See AN649 at section 9.1.5. It was assumed that:
												// 	- CL=10pF
												//	- startup ESR = 5 x run ESR = 5 x 70ohm = 350ohm (max)

#define STATUS0_STCINT				0x01
#define STATUS0_DSRVINT				0x10
#define STATUS0_DACQINT				0x20
#define STATUS0_ERRCMD				0x40
#define STATUS0_CTS					0x80
#define STATUS3_PUP_STATE_mask				0xC0
#define STATUS3_PUP_STATE_BOOTLOADER		0x80
#define STATUS3_PUP_STATE_APPLICATION		0xC0

#define MAX_FLASH_LOAD_PAYLOAD		2048//4096
#define FLASH_LOAD_HEADER			4
uint8_t dummy_buffer[MAX_FLASH_LOAD_PAYLOAD + FLASH_LOAD_HEADER];

static int Si468x_wait_for_status0_bitmask(struct Si468x_status* status, uint8_t bitmask)
{
	struct timespec sys_time;
	long start_time_ms, curr_time_ms;
	
	clock_gettime(CLOCK_MONOTONIC, &sys_time);
	start_time_ms = sys_time.tv_sec * 1000 + sys_time.tv_nsec/1000000;
	
	#ifdef REPLACE_INTERRUPT_WITH_POLLING
		do {
			Si468x_read_reply(NULL, 0, status);
			clock_gettime(CLOCK_MONOTONIC, &sys_time);
			curr_time_ms = sys_time.tv_sec * 1000 + sys_time.tv_nsec/1000000;
			if (curr_time_ms - start_time_ms > WAIT_FOR_STATUS_BIT_MAX_TIMEOUT_MS) {
				printf("Error: timeout in %s\n", __FUNCTION__);
				return -1;
			}
			usleep(WAIT_FOR_STATUS_BIT_POLLING_PERIOD_US);
		} while ((status->status_bytes[0] & bitmask) != bitmask);
	#else 
		// TODO
	#endif
	
	return 0;
}

static int Si468x_send_command_wait_cts_read_reply(uint8_t* cmd_data, uint32_t cmd_size, struct Si468x_status* status, uint8_t* reply_data, uint32_t reply_size)
{
	int ret;
	
	ret = Si468x_send_command(cmd_data, cmd_size);
	if (ret < 0) {
		return ret;
	}
	
	ret = Si468x_wait_for_status0_bitmask(status, STATUS0_CTS);
	if (ret < 0) {
		return ret;
	}
	if (reply_data != NULL) {
		ret = Si468x_read_reply(reply_data, reply_size, status);
		if (ret < 0) {
			return ret;
		}
	}
	
	return 0;
}

int Si468x_powerup()
{
	uint8_t data_out[16] = {
		SI468X_CMD_POWER_UP,
		0x80,	// toggle interrupt when CTS is available
		0x17,	// external crystal; TR_SIZE=0x7 (see AN649 at section 9.1)
		(SI468X_XTAL_STARTUP_BIAS_CURRENT/10),	// see comments in the define
		((SI468X_XTAL_FREQ & 0x000000FF)>>0),
		((SI468X_XTAL_FREQ & 0x0000FF00)>>8),
		((SI468X_XTAL_FREQ & 0x00FF0000)>>16),
		((SI468X_XTAL_FREQ & 0xFF000000)>>24),
		(2*SI468X_XTAL_CL/0.381),	// see AN649 at section 9.3,
		0x10,	// fixed
		0x00,	// fixed
		0x00,	// fixed
		0x00,	// fixed
		((SI468X_XTAL_STARTUP_BIAS_CURRENT/10)/2),	// see AN649 at section 9.2
		0x00,	// fixed
		0x00,	// fixed
	};
	int ret_val;
	
	ret_val = Si468x_send_command(data_out, sizeof(data_out));
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	struct Si468x_status status;
	ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	// data_in[3] contains informations about the current device's state
	if ((status.status_bytes[3] & STATUS3_PUP_STATE_mask) != STATUS3_PUP_STATE_BOOTLOADER) {
		printf("Error in function %s at line %d\n", __FUNCTION__, __LINE__);
		printf("%x %x %x %x\n", status.status_bytes[0], status.status_bytes[1], status.status_bytes[2], status.status_bytes[3]);
		return -1;
	}
	return 0;
}

int Si468x_load_init()
{
	uint8_t data_out[2] = {
		SI468X_CMD_LOAD_INIT,
		0x00
	};
	int ret_val;

	ret_val = Si468x_send_command(data_out, sizeof(data_out));
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	struct Si468x_status status;
	ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	return 0;
}

int Si468x_host_load(uint8_t* img_data, uint32_t len)
{
	uint32_t curr_len;
	int ret_val;
	struct Si468x_status status;
	uint32_t sent_bytes = 0;
	uint32_t total_bytes = len;
	
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
		ret_val = Si468x_send_command(dummy_buffer, FLASH_LOAD_HEADER + curr_len);
		if (ret_val < 0) {
			printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
			return ret_val;
		}

		ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
		if (ret_val < 0) {
			printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
			return ret_val;
		}
		
		sent_bytes += curr_len;
		printf("Progess: %d\%\n", (sent_bytes*100)/total_bytes);

		len -= curr_len;
		img_data += curr_len;
	} while(len > 0);
	
	return 0;
}

int Si468x_boot()
{
	int ret_val;
	uint8_t data_out[2] = {
		SI468X_CMD_BOOT,
		0x00
	};
	
	ret_val = Si468x_send_command(data_out, sizeof(data_out));
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}

	struct Si468x_status status;
	ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	// data_in[3] contains informations about the current device's state
	if ((status.status_bytes[3] & STATUS3_PUP_STATE_mask) != STATUS3_PUP_STATE_APPLICATION) {
		printf("Error in function %s line %d\n", __FUNCTION__, __LINE__);
		printf("%x %x %x %x\n", status.status_bytes[0], status.status_bytes[1], status.status_bytes[2], status.status_bytes[3]);
		return -1;
	}
	return 0;
}

int Si468x_get_part_info(struct Si468x_info *info)
{
	uint8_t data_out[2] = {
		SI468X_CMD_GET_PART_INFO,
		0x00
	};
	Si468x_send_command(data_out, sizeof(data_out));

	struct Si468x_status status;
	int ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	uint8_t data_in[19];
	Si468x_read_reply(data_in, sizeof(data_in), &status);

	if(status.status_bytes[0] & (STATUS0_CTS | STATUS0_ERRCMD)) {
		info->chiprev 	= data_in[0];
		info->romid 	= data_in[1];
		info->part		= ((uint16_t)data_in[5] << 8) | data_in[4];
	}
	return 0;
}

int Si468x_get_sys_state(enum Si468x_exec_mode* mode)
{
	uint8_t data_out[2] = {
		SI468X_CMD_GET_SYS_STATE,
		0x00
	};
	Si468x_send_command(data_out, sizeof(data_out));

	struct Si468x_status status;
	uint8_t data_in[2];
	int ret; 
	
	ret = Si468x_read_reply(data_in, sizeof(data_in), &status);
	if (ret < 0) {
		return ret;
	}
	
	switch(status.status_bytes[0]) {
		case 0:
			*mode = EXEC_BOOTLOADER_MODE;
		case 1:
            *mode = EXEC_FMHD_MODE;
		case 2:
            *mode = EXEC_DAB_MODE;
		default:
            *mode = EXEC_UNKNOWN_MODE;
	}
	
	return 0;
}

int Si468x_dab_get_freq_list(struct Si468x_DAB_freq_list *list)
{
	uint8_t data_out[2] = {
		SI468X_CMD_DAB_GET_FREQ_LIST,
		0x00
	};
	Si468x_send_command(data_out, sizeof(data_out));

	struct Si468x_status status;
	int ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	uint8_t data_in[8];
	Si468x_read_reply(data_in, sizeof(data_in), &status);

	if(status.status_bytes[0] & (STATUS0_CTS | STATUS0_ERRCMD))
	{
		list->num_freqs = data_in[0];
		list->first_freq = (((uint32_t)data_in[7] << 24) |
							((uint32_t)data_in[6] << 16) |
							((uint32_t)data_in[5] << 8) |
							data_in[4]);
		return 0;
	} else {
		printf("Error in %s: status byte[0] = 0x%x\n", __FUNCTION__, status.status_bytes[0]);
		return -1;
	}
}

int Si468x_dab_set_freq_list(void)
{
	uint8_t data_out[8] = {
		SI468X_CMD_DAB_SET_FREQ_LIST,
		1,
		0,
		0,
		0x20,
		0x78,
		0x03,
		0x00,
	};
	Si468x_send_command(data_out, sizeof(data_out));

	struct Si468x_status status;
	int ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	return 0;
}

int Si468x_dab_tune_freq(uint8_t freq)
{
	uint8_t data_out[6] = {
		SI468X_CMD_DAB_TUNE_FREQ,
		0, // INJECTION = 0 (automatic)
		freq,
		0x00,
		0, // ANTCAP = 0 (automatic)
		0,
	};
	Si468x_send_command(data_out, sizeof(data_out));
	
	struct Si468x_status status;
	int ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_STCINT);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	return 0;
}

int Si468x_dab_digrad_status(uint8_t digrad_ack, uint8_t attune, uint8_t stc_ack, struct Si468x_DAB_digrad_status *digrad_status)
{
	uint8_t data_out[2] = {
		SI468X_CMD_DAB_DIGRAD_STATUS,
		0x01 & stc_ack | ((digrad_ack) ? 0x08 : 0x04),
	};
	Si468x_send_command(data_out, sizeof(data_out));

	struct Si468x_status status;
	int ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	uint8_t data_in[19];
	Si468x_read_reply(data_in, sizeof(data_in), &status);

	digrad_status->hardmuteint = !!(data_in[0] & 0x10);
	digrad_status->ficerrint = !!(data_in[0] & 0x08);
	digrad_status->acqint = !!(data_in[0] & 0x04);
	digrad_status->rssihint = !!(data_in[0] & 0x02);
	digrad_status->rssilint = !!(data_in[0] & 0x01);
	
	digrad_status->hardmute = !!(data_in[1] & 0x10);
	digrad_status->ficerr = !!(data_in[1] & 0x08);
	digrad_status->acq = !!(data_in[1] & 0x04);
	digrad_status->valid = !!(data_in[1] & 0x01);

	digrad_status->rssi 			= (int8_t)data_in[2];
	digrad_status->snr 				= data_in[3];
	digrad_status->fic_quality 		= data_in[4];
	digrad_status->cnr				= data_in[5];
	digrad_status->FIB_error_count = data_in[6] | (uint16_t)(data_in[7] << 8);
	digrad_status->tune_freq		= 	(uint32_t)data_in[8] |
								(uint32_t)(data_in[9] << 8) |
								(uint32_t)(data_in[10] << 16)|
								(uint32_t)(data_in[11] << 24);
	digrad_status->tune_index		= data_in[12];
	digrad_status->fft_offset		= (int8_t)data_in[13];
	digrad_status->readantcap		= (uint16_t)data_in[14] | (uint16_t)(data_in[15] << 8);
	digrad_status->culevel			= (uint16_t)data_in[16] | (uint16_t)(data_in[17] << 8);
	digrad_status->fastdect			= data_in[18];

	return 0;
}

int Si468x_dab_get_ensamble_info(struct Si468x_DAB_ensamble_info* ensamble_info)
{	
	uint8_t data_out[2] = {
		SI468X_CMD_DAB_GET_ENSEMBLE_INFO,
		0x00
	};
	Si468x_send_command(data_out, sizeof(data_out));

	struct Si468x_status status;
	int ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	uint8_t data_in[22];
	Si468x_read_reply(data_in, sizeof(data_in), &status);

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

int Si468x_dab_get_time(struct Si468x_DAB_time* dab_time)
{
	uint8_t data_out[2] = {
		SI468X_CMD_DAB_GET_TIME,
		0x00
	};
	Si468x_send_command(data_out, sizeof(data_out));
	
	struct Si468x_status status;
	int ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	uint8_t data_in[7];
	Si468x_read_reply(data_in, sizeof(data_in), &status);
	
	dab_time->year = ((uint16_t)(data_in[1] << 8) | data_in[0]);
	dab_time->month = data_in[2];
	dab_time->day = data_in[3];
	dab_time->hours = data_in[4];
	dab_time->minutes = data_in[5];
	dab_time->seconds = data_in[6];

	return 0;
}

#define DIGITAL_SERVICE_LIST_BUFFER_SIZE		2049
int Si468x_dab_get_digital_service_list(struct Si468x_DAB_digital_service_list_header* list_header, uint8_t** raw_service_list)
{    
    uint8_t data_out[2] = {
		SI468X_CMD_GET_DIGITAL_SERVICE_LIST,
		0x00
	};
	Si468x_send_command(data_out, sizeof(data_out));
	
	struct Si468x_status status;
	int ret_val = Si468x_wait_for_status0_bitmask(&status, STATUS0_CTS);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}

	uint8_t* tmp_buffer = (uint8_t*) malloc(DIGITAL_SERVICE_LIST_BUFFER_SIZE);
	if (tmp_buffer == NULL) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	Si468x_read_reply(tmp_buffer, DIGITAL_SERVICE_LIST_BUFFER_SIZE, &status);

	list_header->service_list_size = (uint16_t)((tmp_buffer[1] << 8) | tmp_buffer[0]);
	list_header->service_list_version = (uint16_t)((tmp_buffer[3] << 8) | tmp_buffer[2]);
	list_header->num_of_services = tmp_buffer[4];
	
	*raw_service_list = (uint8_t*) malloc(list_header->service_list_size);
	if (*raw_service_list == NULL) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		free(tmp_buffer);
		return ret_val;
	}
	memcpy(*raw_service_list, &tmp_buffer[8], list_header->service_list_size);
	free(tmp_buffer);

	return 0;
}

int Si468x_start_digital_service(uint32_t service_id, uint32_t component_id)
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
	struct Si468x_status status;
	
	ret = Si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, NULL, 0);
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}
	
	return 0;
}

int Si468x_dab_get_event_status(uint8_t event_ack, struct Si468x_DAB_event_status* event_status)
{
	uint8_t data_out[2] = {
		SI468X_CMD_DAB_GET_EVENT_STATUS,
		(event_ack & 0x01)
	};
	int ret; 
	struct Si468x_status status;
	uint8_t data_in[4];
	
	ret = Si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, data_in, sizeof(data_in));
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
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

int Si468x_dab_get_audio_info(struct Si468x_DAB_audio_info* audio_info)
{
	uint8_t data_out[2] = {
		SI468X_CMD_DAB_GET_AUDIO_INFO,
		0x01
	};
	int ret; 
	struct Si468x_status status;
	uint8_t data_in[6];
	
	ret = Si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, data_in, sizeof(data_in));
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
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

int Si468x_get_property(uint16_t property, uint16_t* value)
{
	uint8_t data_out[6] = {
		SI468X_CMD_GET_PROPERTY,
		0x01,
		(uint8_t)(0x00FF & property),
		(uint8_t)((0xFF00 & property) >> 8),
	};
	int ret; 
	struct Si468x_status status;
	uint8_t data_in[2];
	
	ret = Si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, data_in, sizeof(data_in));
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}

	*value = ((uint16_t)data_in[0] | ((uint16_t)(data_in[1]) << 8));

	return 0;
}

int Si468x_set_property(uint16_t property, uint16_t value)
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
	struct Si468x_status status;
	
	ret = Si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, NULL, 0);
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}

	return 0;
}

int Si468x_fm_tune_freq(uint16_t freq)
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
	struct Si468x_status status;
	
	ret = Si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, NULL, 0);
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}

	return 0;
}

int Si468x_fm_seek_start(uint8_t seek_up_down, uint8_t wrap)
{
	uint8_t data_out[6] = {
		SI468X_CMD_FM_SEEK_START,
		0x00,	// FORCE_WB = 0; TUNE_MODE = 0; INJECTION = 0
		(seek_up_down & 0x2) | (wrap & 0x1),
		0,
		0,	// ANTCAP = 0 (automatic tuning capacitor)
		0
	};
	int ret;
	struct Si468x_status status;
	
	ret = Si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, NULL, 0);
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}
	
	return 0;
}

int Si468x_fm_rsq_status(struct Si468x_FM_rsq_status* rsq_status)
{
	int ret;
	uint8_t data_out[2] = {
		SI468X_CMD_FM_RSQ_STATUS,
		0x80,	// RSQACK=1; ATTUNE=0; CANCEL=0; STCACK=0
	};
	struct Si468x_status status;
	uint8_t data_in[17];
	
	ret = Si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, data_in, sizeof(data_in));
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}
	
	rsq_status->snrhint = 0;	//TODO
	rsq_status->snrlint = 0;	//TODO
	rsq_status->rssihint = 0;	//TODO
	rsq_status->rssilint = 0;	//TODO
	rsq_status->bltf = 0;	//TODO
	rsq_status->hddetected = 0;	//TODO
	rsq_status->flt_hd_detected = 0;	//TODO
	rsq_status->afcrl = 0;	//TODO
	rsq_status->valid = 0;	//TODO
	rsq_status->read_freq = (uint16_t)(data_in[3] << 8) + (uint16_t)data_in[2];
	rsq_status->freq_off = (int8_t)data_in[4];
	rsq_status->rssi = (int8_t)data_in[5];
	rsq_status->snr = (int8_t)data_in[6];
	rsq_status->mult = data_in[7];
	rsq_status->readantcap = 0;	//TODO
	rsq_status->hdlevel = 0;	//TODO
	rsq_status->filtered_hdlevel = 0;	//TODO
	
	return 0;
}

int Si468x_fm_rds_status(struct Si468x_FM_rds_status* rds_status)
{
	int ret;
	uint8_t data_out[2] = {
		SI468X_CMD_FM_RDS_STATUS,
		0x00,	// STATUSONLY=0; MTFIFO=0; INTACK=0
	};
	struct Si468x_status status;
	uint8_t data_in[16];
	
	ret = Si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, data_in, sizeof(data_in));
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
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

int Si468x_fm_rds_blockcount(struct Si468x_FM_rds_blockcount* blockcount)
{
	int ret;
	uint8_t data_out[2] = {
		SI468X_CMD_FM_RDS_BLOCKCOUNT,
		0x00,	// CLEAR=0
	};
	struct Si468x_status status;
	uint8_t data_in[6];
	
	ret = Si468x_send_command_wait_cts_read_reply(data_out, sizeof(data_out), &status, data_in, sizeof(data_in));
	if (ret < 0) {
		printf("Error (%d) in function %s line %d\n", ret, __FUNCTION__, __LINE__);
		return ret;
	}
	
	blockcount->expected = (uint16_t)(data_in[1] << 8) + (uint16_t)(data_in[0]);
	blockcount->received = (uint16_t)(data_in[3] << 8) + (uint16_t)(data_in[2]);
	blockcount->uncorrectable = (uint16_t)(data_in[5] << 8) + (uint16_t)(data_in[4]);
	
	return 0;
}
