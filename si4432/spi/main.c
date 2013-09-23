#include "spi.h"
#include <stdio.h>

int main()
{
    InitSpiPort();

    printf("main: Write Word\n");
    SpiWrite(0x12,0x34);

    printf("main: Read Word\n");
    uint8_t tmpL;
    tmpL = SpiRead(0x12);
    printf("tmpL is 0x%.2x\n", tmpL);

    printf("main: Burst Write\n");
    uint8_t buf[] = {0x11, 0x22, 0x33};
    SpiBurstWrite(0x12, buf, 3);

    printf("main: Burst Read\n");
    uint8_t readBuf[] = {0x12, 0x34, 0x56, 0x78};
    SpiBurstRead(0x12, readBuf, 4);


    CloseSpi();

    return 0;
}
