#include "spi.h"
#include <vector>
#include <string>
extern "C"
{
#include "c_spi.h"
}


#include <stdio.h>

using std::vector;
using std::string;

void Spi::setDevices(string spi, string spiInterrupt)
{
    _spi = spi;
    _spiInterrupt = spiInterrupt;
}

void Spi::chipOpen()
{
   c_open(_spi.c_str(), _spiInterrupt.c_str());
}

void Spi::chipClose()
{
    c_close();
}

void Spi::chipWrite(unsigned char addr, unsigned char value)
{
    c_write(addr, value);
}

unsigned char Spi::chipRead(unsigned char addr)
{
    unsigned char value;
    value = c_read(addr);
    return value;
}

void Spi::burstWrite(unsigned char addr, vector<unsigned char> data)
{
    if (true == data.empty())
    {
        return;
    }

    c_burstWrite(addr, &data[0], (unsigned char)data.size());

}
vector<unsigned char> Spi::burstRead(unsigned char addr, unsigned char length)
{
    vector<unsigned char>rtnData(length);
    c_burstRead(addr, &rtnData[0], length);
    return rtnData;
}

bool Spi::getInterrupt()
{
    unsigned char status;
    status = c_getInterrupt();
    if ( 0 == status)
    {
        return true;
    }
    return false;
}

