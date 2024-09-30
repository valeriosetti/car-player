#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include "si468x_utils.h"

int read_file_to_buffer(char *path, unsigned char **out_buf, size_t *out_buf_size)
{
    FILE* file;
    int fd;
    size_t read_bytes;
    struct stat f_stat;
    int ret;

    file = fopen(path, "rb");
    if (file == NULL) {
        return -EINVAL;
    }

    fd = fileno(file);

    ret = fstat(fd, &f_stat);
    if (ret < 0) {
        goto exit;
    }

    *out_buf_size = f_stat.st_size;
    *out_buf = malloc(*out_buf_size);
    if (*out_buf == NULL) {
        return -ENOMEM;
    }

    read_bytes = fread(*out_buf, 1, *out_buf_size, file);
    if (read_bytes != *out_buf_size) {
        ret = -EIO;
    }

exit:
    fclose(file);
    return ret;
}
