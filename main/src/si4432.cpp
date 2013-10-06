#include "si4432.h"
#include <vector>
#include <unistd.h>

#include <stdio.h>

using std::vector;

#define SI4432_CRYSTAL_CAPACITANCE 0xB4

#define TxGpioSetting() _spi->chipWrite(0x0e, 0x02)
#define RxGpioSetting() _spi->chipWrite(0x0e, 0x01)
#define IdleGpioSetting() _spi->chipWrite(0x0e, 0x01)

void Si4432::setSpi(Spi * spi)
{
    _spi = spi;
}

void Si4432::reset()
{
    _spi->chipRead(0x03);
    _spi->chipRead(0x04);

    //reset si4432 chip
    _spi->chipWrite(0x07, 0x80);
    printf("si4432.cpp:reset():reset chip.......\n");

    unsigned char timeCount = 0;
    while(true)
    {
        if ( true == _spi->getInterrupt())
        {
            break;
        }
        if (100 < timeCount)
        {
            break;
        }

        timeCount++;

        usleep(20000);
    }

    if (false == _spi->getInterrupt())
    {
        printf("si4432.cpp:reset():reset chip fail!\n");
        init();
        setIdleMode();
    }

    _spi->chipRead(0x03);
    _spi->chipRead(0x04);

}

void Si4432::init()
{
    _spi->chipWrite(0x75, 0x53);
    _spi->chipWrite(0x76, 0x4B);
    _spi->chipWrite(0x77, 0x00);

    _spi->chipWrite(0x6e, 0x09);
    _spi->chipWrite(0x6f, 0xd5);
    _spi->chipWrite(0x70, 0x2C);
    _spi->chipWrite(0x58, 0x80);

    _spi->chipWrite(0x6d, 0x1f);

    _spi->chipWrite(0x72, 0x20);

    _spi->chipWrite(0x1c, 0x2c);
    _spi->chipWrite(0x20, 0x41);
    _spi->chipWrite(0x21, 0x60);
    _spi->chipWrite(0x22, 0x27);
    _spi->chipWrite(0x23, 0x52);
    _spi->chipWrite(0x24, 0x00);
    _spi->chipWrite(0x25, 0x04);
    _spi->chipWrite(0x1d, 0x40);
    _spi->chipWrite(0x1e, 0x0a);
    _spi->chipWrite(0x2a, 0x0f);
    _spi->chipWrite(0x1f, 0x03);
    _spi->chipWrite(0x69, 0x60);

    _spi->chipWrite(0x34, 0x0c);
    _spi->chipWrite(0x35, 0x2a);

    _spi->chipWrite(0x33, 0x02);

    _spi->chipWrite(0x36, 0x2d);
    _spi->chipWrite(0x37, 0xd4);

    _spi->chipWrite(0x30, 0x8d);
    _spi->chipWrite(0x32, 0x00);

    _spi->chipWrite(0x71, 0x63);

    _spi->chipWrite(0x0b, 0xca);
    _spi->chipWrite(0x0c, 0xca);
    _spi->chipWrite(0x0d, 0xca);

    _spi->chipWrite(0x09, SI4432_CRYSTAL_CAPACITANCE);
}

void Si4432::setRxMode()
{
    _spi->chipRead(0x03);
    _spi->chipRead(0x04);

    RxGpioSetting();
    _spi->chipWrite(0x07, 0x05);
}

void Si4432::setTxMode()
{
    _spi->chipRead(0x03);
    _spi->chipRead(0x04);

    TxGpioSetting();
    _spi->chipWrite(0x07, 0x09);
}

void Si4432::setIdleMode()
{
    //disable all interrupt
    _spi->chipWrite(0x05, 0x00);
    _spi->chipWrite(0x06, 0x00);

    _spi->chipRead(0x03);
    _spi->chipRead(0x04);

    IdleGpioSetting();
    _spi->chipWrite(0x07, 0x01);

    _spi->chipRead(0x03);
    _spi->chipRead(0x04);
}

void Si4432::fifoSend(vector<unsigned char> data)
{
    setIdleMode();

    //printf("si4432.cpp:fifoSend():prepare to send!\n");

//    /************DEBUG*****************/
//    printf("si4432.cpp:send data is:");
//    for (int i = 0; i < (int)data.size(); i++)
//    {
//        printf(" 0x%.2x ", data[i]);
//    }
//    printf("\n");
//    /*********************************/

    //clear tx fifo
    _spi->chipWrite(0x08, 0x01);
    _spi->chipWrite(0x08, 0x00);

    //burst data to fifo
    _spi->chipWrite(0x3e, (unsigned char)data.size());
    _spi->burstWrite(0x7f, data);

    //only enable package sent event interrupt
    _spi->chipWrite(0x05, 0x04);
    _spi->chipWrite(0x06, 0x00);

    setTxMode();
    //printf("si4432.cpp:fifoSend():sending......\n");

    unsigned char timeCount = 0;
    while(true)
    {
        if ( true == _spi->getInterrupt())
        {
            break;
        }
        if (100 < timeCount)
        {
            break;
        }

        timeCount++;

        usleep(20000);
    }

    if (false == _spi->getInterrupt())
    {
        printf("si4432.cpp:fifoSend():send fail!\n");
        //clear tx fifo
        _spi->chipWrite(0x08, 0x01);
        _spi->chipWrite(0x08, 0x00);
    }


    //enable package recv event and crc error event interrupt
    _spi->chipWrite(0x05, 0x03);
    _spi->chipWrite(0x06, 0x00);

    setRxMode();
    _spi->chipRead(0x03);
    _spi->chipRead(0x04);

    //printf("si4432.cpp:fifoSend():send end!\n");
}

vector<unsigned char> Si4432::fifoRead()
{
    unsigned char length;
    length = _spi->chipRead(0x4b);

    /*********DEBUG*************/
    //printf("si4432.cpp:fifoRead():fifo data length is %d\n", length);
    /***************************/

    vector<unsigned char> data;
    data = _spi->burstRead(0x7f, length);

    //clear rx fifo
    _spi->chipWrite(0x08, 0x02);
    _spi->chipWrite(0x08, 0x00);


    return data;
}

bool Si4432::isReceived()
{
    bool status = false;
    //wait for interrupt or wait enough time
    unsigned short timeCount = 0;
    while(true)
    {
        if ( true == _spi->getInterrupt())
        {
            break;
        }
        if (150 < timeCount)
        {
            break;
        }

        timeCount++;

        usleep(20000);
    }


    if ( true == _spi->getInterrupt())
    {
        unsigned char itStatus1;
        //unsigned char itStatus2;
        itStatus1 = _spi->chipRead(0x03);
        _spi->chipRead(0x04);

        if (0x01 == (itStatus1 & 0x01))
        {
            //clear rx fifo
            _spi->chipWrite(0x08, 0x02);
            _spi->chipWrite(0x08, 0x00);
            printf("si4432.cpp:isReceived():receive error!\n");
        }
        if (0x02 == (itStatus1 & 0x02))
        {
            printf("si4432.cpp:isReceived():receive OK!\n");
            status = true;
        }
    }

    setIdleMode();

    return status;
}



