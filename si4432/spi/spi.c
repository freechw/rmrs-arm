//#include "spi.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>



static const char *device = "/dev/spidev1.0"; //The filename of SPI device

uint8_t mode;
uint8_t bits = 16;

uint32_t speed = 5000000;

uint16_t delay;

static int fd;

static void pabort(const char *s)
{
    perror(s);

    abort();
}


void InitSpiPort()
{
    int status = 0;

//    mode |= SPI_CPHA;
//    mode |= SPI_CPOL;
//    mode &= ~SPI_CS_HIGH;
    mode = SPI_MODE_0;

    fd = open(device, O_RDWR);
    if (0 > fd)
    {
        pabort("can't open device");
    }

    status = ioctl(fd, SPI_IOC_WR_MODE, &mode);//set mode of spi
    if (-1 == status)
    {
        pabort("can't set spi mode");
    }

    status = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);//set bits per word
    if (-1 == status)
    {
        pabort("can't set bits");
    }

    status = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);//set max speed of spi
    if (-1 == status)
    {
        pabort("can't set max speed");
    }

}


void SpiWrite(uint8_t addr, uint8_t value)
{
    uint16_t tx[1];
    tx[0] = ((uint16_t)(addr | 0x80) << 8)|((uint16_t)(value));

    uint16_t rx[1];
    rx[0] = 0x0000;

    struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)tx,
            .rx_buf = (unsigned long)rx,
            .len = 2,
            .speed_hz = speed,
            .bits_per_word = bits,
    };

    int status;
    status = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (0 > status)
    {
        pabort("can't send spi message");
    }

    /***********DEBUG*******************/
    printf("SpiWrite: rx buf is 0x%.2x \n", rx[0]);
    /***********************************/
}

uint8_t SpiRead(uint8_t addr)
{
    uint16_t tx[1];
    tx[0] = ((uint16_t)(addr) << 8) | (0x00aa);

    uint16_t rx[1];
    rx[0] = 0x0000;

    struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)tx,
            .rx_buf = (unsigned long)rx,
            .len = 2,
            .speed_hz = speed,
            .bits_per_word = bits,
    };

    int status;
    status = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (0 > status)
    {
        pabort("can't read spi message");
    }

    uint8_t value = (rx[0] & 0xff);

    /***************DEBUG******************/
    printf("SpiRead: rx is 0x%.2x \n", rx[0]);
    /**************************************/

    return value;
}

void SpiBurstWrite(uint8_t addr, uint8_t buf[], uint8_t length)
{
    uint8_t tx[1+length];
    tx[0] = (addr | 0x80);
    memcpy(&tx[1], buf, length);

    uint8_t rx[1+length];

    struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)tx,
            .rx_buf = (unsigned long)rx,
            .len = 1+length,
            .speed_hz = speed,
            .bits_per_word = 8,
    };

    int status;
    status = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (0 > status)
    {
        pabort("can't send burst message");
    }

    /*****************DEBUG**************/
    printf("SpiBurstWrite: rx buf is");
    for(int i = 0; i < 1+length; i++)
    {
        printf(" 0x%.2x ", rx[i]);
    }
}

void SpiBurstRead(uint8_t addr, uint8_t buf[], uint8_t length)
{
    uint8_t tx[1+length];
    memset(tx, 0, 1+length);
    tx[0] = addr;

    uint8_t rx[1+length];

    struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)tx,
            .rx_buf = (unsigned long)rx,
            .len = 1+length,
            .speed_hz = speed,
            .bits_per_word = 8,
    };

    int status;
    status = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (0 > status)
    {
        pabort("can't send bursr message");
    }

    memcpy(buf, &rx[1], length);

    /*************DEBUG************/
    printf("SpiBurstRead: rx buf is");
    for (int i = 0; i < 1+length; i++)
    {
        printf(" 0x%.2x ", rx[i]);
    }
    printf("\n");
    /******************************/
}

void CloseSpi()
{
    close(fd);
}



