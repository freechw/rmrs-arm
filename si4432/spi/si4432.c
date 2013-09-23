//#include "si4432.h"
#include "spi.h"
#include "si4432interrupt.h"
#include <stdio.h>

#include <unistd.h>//for usleep();

#define SI4432_CRYSTAL_CAPACITANCE 0xB4

#define TxGpioSetting() SpiWrite(0x0e, 0x02)
#define RxGpioSetting() SpiWrite(0x0e, 0x01)
#define IdleGpioSetting() SpiWrite(0x0e, 0x01)


unsigned char ItStatus1, ItStatus2;

void Si4432Reset()
{
    ItStatus1 = SpiRead(0x03);
    ItStatus2 = SpiRead(0x04);

    //reset si4432 chip
    SpiWrite(0x07, 0x80);
    printf("Si4432: Reset Chip......\n");

    while(Si4432GetInterrupt());

    ItStatus1 = SpiRead(0x03);
    ItStatus1 = SpiRead(0x04);
}

void Si4432Init()
{

    SpiWrite(0x75, 0x53);
    SpiWrite(0x76, 0x4B);
    SpiWrite(0x77, 0x00);

    SpiWrite(0x6e, 0x09);
    SpiWrite(0x6f, 0xd5);
    SpiWrite(0x70, 0x2C);
    SpiWrite(0x58, 0x80);

    SpiWrite(0x6d, 0x1f);

    SpiWrite(0x72, 0x20);

    SpiWrite(0x1c, 0x2c);
    SpiWrite(0x20, 0x41);
    SpiWrite(0x21, 0x60);
    SpiWrite(0x22, 0x27);
    SpiWrite(0x23, 0x52);
    SpiWrite(0x24, 0x00);
    SpiWrite(0x25, 0x04);
    SpiWrite(0x1d, 0x40);
    SpiWrite(0x1e, 0x0a);
    SpiWrite(0x2a, 0x0f);
    SpiWrite(0x1f, 0x03);
    SpiWrite(0x69, 0x60);

    SpiWrite(0x34, 0x0c);
    SpiWrite(0x35, 0x2a);

    SpiWrite(0x33, 0x02);

    SpiWrite(0x36, 0x2d);
    SpiWrite(0x37, 0xd4);

    SpiWrite(0x30, 0x8d);
    SpiWrite(0x32, 0x00);

    SpiWrite(0x71, 0x63);

    SpiWrite(0x0b, 0xca);
    SpiWrite(0x0c, 0xca);
    SpiWrite(0x0d, 0xca);

    SpiWrite(0x09, SI4432_CRYSTAL_CAPACITANCE);
}

void Si4432SetRxMode()
{
    ItStatus1 = SpiRead(0x03);
    ItStatus2 = SpiRead(0x04);

    RxGpioSetting();

    SpiWrite(0x07, 0x05);
}

void Si4432SetTxMode()
{
    ItStatus1 = SpiRead(0x03);
    ItStatus2 = SpiRead(0x04);

    TxGpioSetting();

    SpiWrite(0x07, 0x09);
}

void Si4432SetIdleMode()
{
    SpiWrite(0x07, 0x01);

    IdleGpioSetting();

    ItStatus1 = SpiRead(0x03);
    ItStatus1 = SpiRead(0x04);
}

void Si4432FifoSend(unsigned char buf[], unsigned char length)
{
    Si4432SetIdleMode();

    printf("Si4432: Prepare to send!\n");

    /**********DEBUG******************/
    printf("Si4432: Send data is:");
    for (int i = 0; i < length; i++)
    {
        printf(" 0x%.2x ", buf[i]);
    }
    printf("\n");
    /*********************************/

    SpiWrite(0x3e, length);
    SpiBurstWrite(0x7f, buf, length);

    SpiWrite(0x05, 0x04);
    SpiWrite(0x06, 0x00);

    Si4432SetTxMode();
    printf("Si4432: sending...\n");

    //while NIRQ and Timer
    while(Si4432GetInterrupt());


    SpiWrite(0x05, 0x03);
    SpiWrite(0x06, 0x00);
    Si4432SetRxMode();
    ItStatus1 = SpiRead(0x03);
    ItStatus2 = SpiRead(0x04);

    printf("Si4432: Send End!\n");
}

int main()
{
    InitSpiPort();

    Si4432InitInterrupt();

    Si4432Reset();

    Si4432Init();

    Si4432SetRxMode();

    unsigned char message[] = {0x12, 0x7d, 0xff, 0x12};
    Si4432FifoSend(message, 4);

    while(1)
    {
        if (0 == Si4432GetInterrupt())
        {
            printf("Main:Si4432 Interrupt!!!\n");
        }
    }


    return 0;

}
