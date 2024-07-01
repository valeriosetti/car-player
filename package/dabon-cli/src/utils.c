#include "utils.h"
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include <unistd.h>

#define MAX_DEBUG_CONTENT_SIZE        32

int read_file_to_buffer(char *path, unsigned char **out_buf, size_t *out_buf_size)
{
    FILE *fp;
    int ret;

    fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    *out_buf_size = ftell(fp);
    fseek(fp, 0, SEEK_SET); 

    *out_buf = malloc(*out_buf_size);
    fread(*out_buf, *out_buf_size, 1, fp);

    fclose(fp);
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
    FILE *fp = fopen(path, "rt");
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

    return 0;
}
