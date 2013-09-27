#ifndef __CLASS_SPI__
#define __CLASS_SPI__

#include <vector>
#include <string>


class Spi
{
    public:
        void setDevices(std::string spi, std::string spiInterrupt);
        void chipOpen();
        void chipClose();
        void chipWrite(unsigned char addr, unsigned char value);
        unsigned char chipRead(unsigned char addr);
        void burstWrite(unsigned char addr, std::vector<unsigned char> data);
        std::vector<unsigned char> burstRead(unsigned char addr, unsigned char length);
        bool getInterrupt();
    private:
        std::string _spi;
        std::string _spiInterrupt;
};


#endif
