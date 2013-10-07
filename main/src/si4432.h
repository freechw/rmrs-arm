#ifndef __CLASS_SI4432__
#define __CLASS_SI4432__

#include <vector>
#include "spi.h"

class Si4432
{
    public:
        void setSpi(Spi * spi);
        void reset();
        void init(short syncword);
        void setTxMode();
        void setRxMode();
        void setIdleMode();
        void fifoSend(std::vector<unsigned char> data);
        std::vector<unsigned char> fifoRead();
        bool isReceived();
    private:
        Spi * _spi;
};


#endif
