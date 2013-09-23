#include "si4432interrupt.h"
#include <stdio.h>
#include <fcntl.h>

static int fd;

void Si4432InitInterrupt()
{
    fd = open("/dev/my_led", O_RDWR);
    if (0 > fd)
    {
        printf("Si4432: Open Interrut Error\n");
    }
}

void Si4432CloseInterrupt()
{
    close(fd);
}

unsigned char Si4432GetInterrupt()
{
    unsigned char status;
    read(fd, &status, 1);

    if(0 == status)
    {
        return 0;
    }

    return 1;
}

