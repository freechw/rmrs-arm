#include "c_spi.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>

static unsigned char mode;
static unsigned char bits = 16;
static unsigned int speed = 5000000;
//static unsigned short delay;
static int spiFd;
static int spiInterruptFd;

void c_open(const char *spiDevice, const char *spiInterruptDevice)
{
    mode = SPI_MODE_0;

    spiFd = open(spiDevice, O_RDWR);
    if (0 > spiFd)
    {
        printf("c_spi.c:c_open():open spi device fail!\n");
    }

    int status = 0;
    status = ioctl(spiFd, SPI_IOC_WR_MODE, &mode);
    if (-1 == status)
    {
        printf("c_spi.c:c_open():set spi mode fail!\n");
    }

    status = ioctl(spiFd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (-1 == status)
    {
        printf("c_spi.c:c_open():set spi bits fail!\n");
    }

    status = ioctl(spiFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (-1 == status)
    {
        printf("c_spi.c:c_open():set spi speed fail!\n");
    }

    spiInterruptFd = open(spiInterruptDevice, O_RDWR);
    if (0 > spiInterruptFd)
    {
        printf("c_spi.c:c_open():open interrupt device fail!\n");
    }
}

void c_close()
{
    close(spiFd);
    close(spiInterruptFd);
}

void c_write(unsigned char addr, unsigned char value)
{
    unsigned short tx[1];
    tx[0] = ((unsigned short)(addr | 0x80) << 8) | ((unsigned short)(value));

    unsigned short rx[1];
    rx[0] = 0x0000;

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 2,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    int status;
    status = ioctl(spiFd, SPI_IOC_MESSAGE(1), &tr);
    if (0 > status)
    {
        printf("c_spi.c:c_write():send spi message fail!\n");
    }
}

unsigned char c_read(unsigned char addr)
{
    unsigned short tx[1];
    tx[0] = ((unsigned short)(addr) << 8) | (0x00aa);

    unsigned short rx[1];
    rx[0] = 0x0000;

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 2,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    int status;
    status = ioctl(spiFd, SPI_IOC_MESSAGE(1), &tr);
    if (0 > status)
    {
        printf("c_spi.c:c_read():read spi message fail!\n");
    }

    unsigned char value = (rx[0] & 0xff);

    return value;
}

void c_burstWrite(unsigned char addr, unsigned char buf[], unsigned char length)
{
    unsigned char tx[1 + length];
    tx[0] = (addr | 0x80);
    memcpy(&tx[1], buf, length);

//    /***********DEBUG*************/
//    printf("c_spi.c:c_burstWrite():burst data is");
//    for (int i = 0; i < length; i++)
//    {
//        printf(" 0x%.2x ", tx[1+i]);
//    }
//    printf("\n");
//    /*****************************/

    unsigned char rx[1 + length];

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 1 + length,
        .speed_hz = speed,
        .bits_per_word = 8,
    };

    int status;
    status = ioctl(spiFd, SPI_IOC_MESSAGE(1), &tr);
    if (0 > status)
    {
        printf("c_spi.c:c_burstWrite():send burst message fail!\n");
    }
}

void c_burstRead(unsigned char addr, unsigned char buf[], unsigned char length)
{
    unsigned char tx[1 + length];
    memset(tx, 0, 1+length);
    tx[0] = addr;

    unsigned char rx[1 + length];

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 1 + length,
        .speed_hz = speed,
        .bits_per_word = 8,
    };

    int status;
    status = ioctl(spiFd, SPI_IOC_MESSAGE(1), &tr);
    if (0 > status)
    {
        printf("c_spi.c:c_burstRead():read burst message fail!\n");
    }

    memcpy(buf, &rx[1], length);

}

unsigned char c_getInterrupt()
{
    unsigned char status;
    read(spiInterruptFd, &status, 1);

    if (0 == status)
    {
        return 0;
    }

    return 1;
}

