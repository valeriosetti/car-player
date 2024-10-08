#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#define ARRAY_SIZE(_x_) (sizeof(_x_)/sizeof(_x_[0]))
#define BIT_MASK(a, b) (((unsigned) -1 >> (31 - (b))) & ~((1U << (a)) - 1))
#define BYTE_0(x)       ((x & 0x000000FF) >> 0)
#define BYTE_1(x)       ((x & 0x0000FF00) >> 8)
#define BYTE_2(x)       ((x & 0x00FF0000) >> 16)
#define BYTE_3(x)       ((x & 0xFF000000) >> 24)

#define ERROR(fmt, ...)     fprintf(stderr, "Error (%s-%d): " fmt "\n", \
                                    __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define WARN(fmt,...)       fprintf(stdout, "Warn (%s-%d): " fmt "\n", \
                                    __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define INFO(fmt,...)       fprintf(stdout, "Info (%s-%d): " fmt "\n", \
                                    __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ERROR_TRACE()           fprintf(stderr, "Error: %s-%d\n", __FUNCTION__, __LINE__)
#define DUMP_STATUS(status)     fprintf(stderr, "Status: [0] %x [1] %x [2] %x [3] %x\n", \
                                        status.byte[0], status.byte[1], \
                                        status.byte[2], status.byte[3])
#define ERROR_TRACE_DUMP_STATUS(status)     ERROR_TRACE(); \
                                            DUMP_STATUS()

#define PRINT(fmt, ...)     fprintf(stdout, fmt "\n", ##__VA_ARGS__)
#define TRACE()     fprintf(stdout, "%s-%d\n", __FUNCTION__, __LINE__)

int read_file_to_buffer(char *path, unsigned char **out_buf, size_t *out_buf_size);

#endif //_UTILS_H_
