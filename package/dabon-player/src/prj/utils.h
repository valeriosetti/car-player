#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define LOG(fmt, ...)   fprintf(stdout, fmt "\n", ##__VA_ARGS__)
#define TRACE()		fprintf(stdout, "%s:%d\n", __FUNCTION__, __LINE__)

#endif /* UTILS_H */