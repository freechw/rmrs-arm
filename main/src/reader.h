#ifndef __CLASS_READER__
#define __CLASS_READER__

#include <vector>
#include <list>
#include <map>
#include <pthread.h>
#include <semaphore.h>
#include "unit.h"
#include "si4432.h"

#include <stdio.h>

class Reader
{
    public:
        void setUnitMap(std::map<short, Unit * > * pUnitMap, sem_t * mapLock);
        void setSi4432(Si4432 * pSi4432);
        void start();
    private:
        void sendReadCommand(short unitId, std::vector<int> meterIds);
        void sendUploadCommand(short unitId, int meterId);
        void sendClearCommand(short unitId);
        bool isCurrent(std::vector<unsigned char> rdBackData, std::vector<int> currentMeters);
        std::map<short, Unit *> * _pUnitMap;
        sem_t * _mapLock;
        friend void* readerProcess(void * args);
        void readCycle();
        void DoRead(Unit * pUnit);
        pthread_t _pthReader;
        Si4432 * _pSi4432;
        short _lastUnitId;
};


#endif
