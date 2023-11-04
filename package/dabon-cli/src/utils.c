#include "utils.h"
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include <unistd.h>

#define INPUT_LINE_MAX_LEN	32

#define MAX_DEBUG_CONTENT_SIZE		32

int get_int_from_stdin(int min_val, int max_val, enum input_format format,  int* output)
{
	char input_line[INPUT_LINE_MAX_LEN];
	int input_value;

	if (fgets(input_line, INPUT_LINE_MAX_LEN, stdin) == NULL) {
		return -1;
	}
	if (format == INPUT_AS_DEC) {
		if (sscanf(input_line, "%d", &input_value) != 1) {
			printf("Error: invalid input");
			return -2;
		}
	} else {
		if (sscanf(input_line, "0x%x", &input_value) != 1) {
			printf("Error: invalid input");
			return -2;
		}
	}
	if ((input_value < min_val) || (input_value > max_val)) {
		printf("Error: input out of range");
		return -3;
	}
	
	*output = input_value;
	return 0;
}

int sysfs_write_int(char* path, int value) 
{
	int ret;
	FILE* fp = fopen(path, "wt");
	if (fp == NULL) {
		printf("Error: unable to open path %s", path);
		return -1;
	}
	
	ret = fprintf(fp, "%d", value);
	if (ret < 0) {
		printf("Error writing file %s", path);
		goto Exit;
	}

Exit:
	fclose(fp);
	
	return ret;
}

int sysfs_write_string(char* path, char* data)
{
	int ret;
	FILE* fp = fopen(path, "wt");
	if (fp == NULL) {
		printf("Error: unable to open path %s", path);
		return -1;
	}
	
	ret = fprintf(fp, "%s", data);
	if (ret < 0) {
		printf("Error writing file %s", path);
		goto Exit;
	}

Exit:
	fclose(fp);
	
	return ret;
}

int sysfs_read_int(char* path, int* value) 
{
	int ret;
	FILE* fp = fopen(path, "rt");
	if (fp == NULL) {
		printf("Error: unable to open path %s", path);
		return -1;
	}
	
	ret = fscanf(fp, "%d", &value);
	if (ret < 0) {
		printf("Error reading file %s", path);
		goto Exit;
	}

Exit:
	fclose(fp);
	
	return ret;
}

int dump_array(uint8_t* data, uint32_t length)
{
	int i;
	uint32_t dbg_size = (length > MAX_DEBUG_CONTENT_SIZE) ? MAX_DEBUG_CONTENT_SIZE : length;
	for (i = 0; i < dbg_size; i++) {
		printf("[%d] 0x%x\n", i, data[i]);
	}
}
