#ifndef _SI468x_PLATFORM_H_
#define _SI468x_PLATFORM_H_

int Si468x_platform_init();
int Si468x_platform_deinit();

int Si468x_send_command(uint8_t* data, uint32_t size);
int Si468x_read_reply(uint8_t* data, uint32_t size, struct Si468x_status* status);

int Si468x_gpio_assert_reset();
int Si468x_gpio_deassert_reset();
uint8_t Si468x_gpio_get_int_status();

#endif // _SI468x_PLATFORM_H_
