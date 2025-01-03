#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
// #include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <gpiod.h>

#include "si468x.h"
#include "si468x_platform.h"

#define GPIO_CHIP_PATH        "/dev/gpiochip1"
#define RESET_PIN        95
#define INT_PIN            89

#define SPI_DEVICE        "/dev/spidev0.0"
#define SPI_SPEED        10000000
#define SPI_BITS        8
#define SPI_DELAY        0

struct gpiod_chip *gpio_chip;
struct gpiod_line *reset_line;
struct gpiod_line *int_line;

int spi_fd;

int si468x_spi_init()
{
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = SPI_BITS;
    uint32_t speed = SPI_SPEED;

    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        ERROR("SPI open failed");
        return -1;
    }

    if (ioctl(spi_fd, SPI_IOC_RD_MODE, &mode) < 0) {
        ERROR("SPI_IOC_RD_MODE failed");
        return -1;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        ERROR("SPI_IOC_WR_MODE failed");
        return -1;
    }

    if (ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) {
        ERROR("SPI_IOC_RD_BITS_PER_WORD failed");
        return -1;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        ERROR("SPI_IOC_WR_BITS_PER_WORD failed");
        return -1;
    }

    if (ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
        ERROR("SPI_IOC_RD_MAX_SPEED_HZ failed");
        return -1;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        ERROR("SPI_IOC_WR_MAX_SPEED_HZ failed");
        return -1;
    }

    return 0;
}

int si468x_spi_close()
{
    return 0;
}

int si468x_send_command(uint8_t* data, uint32_t size)
{
    struct spi_ioc_transfer spi_transfer;
    int ret;
    if (data == NULL) {
        return -1;
    }

    memset(&spi_transfer, 0, sizeof(spi_transfer));
    spi_transfer.tx_buf = (unsigned long) data;
    spi_transfer.rx_buf = (unsigned long) NULL;
    spi_transfer.len = size;
    ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi_transfer);
    if (ret < 0) {
        ERROR("SPI ioctl");
        return ret;
    }
    return 0;
}

#define SI468X_CMD_RD_REPLY                0x00
int si468x_read_reply(uint8_t* data, uint32_t size, struct si468x_status* status)
{
    struct spi_ioc_transfer spi_transfer[3];
    int transactions_count = 0;
    int ret; 
    uint8_t data_out = SI468X_CMD_RD_REPLY;
    memset(spi_transfer, 0, 3*sizeof(struct spi_ioc_transfer));
    spi_transfer[0].tx_buf = (unsigned long) &data_out;
    spi_transfer[0].rx_buf = (unsigned long) NULL;
    spi_transfer[0].len = sizeof(uint8_t);
    transactions_count++;
    spi_transfer[1].tx_buf = (unsigned long) NULL;
    spi_transfer[1].rx_buf = (unsigned long) status;
    spi_transfer[1].len = sizeof(struct si468x_status);
    transactions_count++;
    if (data != NULL) {
        spi_transfer[2].tx_buf = (unsigned long) NULL;
        spi_transfer[2].rx_buf = (unsigned long) data;
        spi_transfer[2].len = size;
        transactions_count++;
    }
    ret = ioctl(spi_fd, SPI_IOC_MESSAGE(transactions_count), &spi_transfer);
    if (ret < 0) {
        ERROR("SPI ioctl");
        return ret;
    }

    return 0;
}

int si468x_gpio_init()
{
    int req;
    gpio_chip = gpiod_chip_open(GPIO_CHIP_PATH);
    if (!gpio_chip) {
        ERROR("Error: unable to open %s", GPIO_CHIP_PATH);
        return -1;
    }

    reset_line = gpiod_chip_get_line(gpio_chip, RESET_PIN);
    if (!reset_line) {
        gpiod_chip_close(gpio_chip);
        return -1;
    }

    req = gpiod_line_request_output(reset_line, "dabon-cli", 1);
    if (req) {
        gpiod_chip_close(gpio_chip);
        return -1;
    }

    int_line = gpiod_chip_get_line(gpio_chip, INT_PIN);
    if (!int_line) {
        gpiod_chip_close(gpio_chip);
        return -1;
    }

    req = gpiod_line_request_input(int_line, "dabon-cli");
    if (req) {
        gpiod_chip_close(gpio_chip);
        return -1;
    }
    return 0;
}

int si468x_gpio_close() 
{
    return 0;
}

int si468x_gpio_assert_reset()
{
    gpiod_line_set_value(reset_line, 0);

    return 0;
}

int si468x_gpio_deassert_reset()
{
    gpiod_line_set_value(reset_line, 1);
    return 0;
}

uint8_t si468x_gpio_get_int_status()
{
    return gpiod_line_get_value(int_line);
}

int si468x_platform_init()
{
    int ret;

    ret = si468x_spi_init();
    if (ret)
        return ret;

    ret = si468x_gpio_init();
    if (ret)
        return ret;

    return 0;
}

int si468x_platform_deinit()
{
    int ret;

    ret = si468x_gpio_close();
    if (ret)
        return ret;

    ret = si468x_spi_close();
    if (ret)
        return ret;

    return 0;
}
