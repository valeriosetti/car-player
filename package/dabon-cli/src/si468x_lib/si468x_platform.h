#ifndef _SI468x_PLATFORM_H_
#define _SI468x_PLATFORM_H_

#include <stdint.h>
#include "si468x_int.h"

int si468x_platform_init();
int si468x_platform_deinit();

int si468x_send_command(uint8_t* data, uint32_t size);
int si468x_read_reply(uint8_t* data, uint32_t size, struct si468x_status* status);

int si468x_gpio_assert_reset();
int si468x_gpio_deassert_reset();
uint8_t si468x_gpio_get_int_status();

#endif // _SI468x_PLATFORM_H_
