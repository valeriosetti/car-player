#ifndef _UTILS_H_
#define _UTILS_H_

#include "stdint.h"

#define ARRAY_SIZE(_x_) (sizeof(_x_)/sizeof(_x_[0]))
#define BIT_MASK(a, b) (((unsigned) -1 >> (31 - (b))) & ~((1U << (a)) - 1))

enum input_format {
	INPUT_AS_DEC,
	INPUT_AS_HEX,
};
int get_int_from_stdin(int min_val, int max_val, enum input_format format,  int* output);

int sysfs_write_int(char* path, int value);
int sysfs_write_string(char* path, char* data);
int sysfs_read_int(char* path, int* value);
int dump_array(uint8_t* data, uint32_t length);

#endif //_UTILS_H_
