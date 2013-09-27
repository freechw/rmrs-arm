#ifndef __CLASS_READER__
#define __CLASS_READER__

#include <vector>
#include <list>
#include <map>
#include <pthread.h>
#include <semaphore.h>
#include "unit.h"

#include <stdio.h>

class Reader
{
    public:
        void setUnitMap(std::map<short, Unit * > * pUnitMap, sem_t * mapLock);
        void start();
    private:
        void sendReadCommand(short unitId, std::vector<int> meterIds);
        void sendUploadCommand(short unitId, int meterId);
        void sendClearCommand(short unitId);
        std::map<short, Unit *> * _pUnitMap;
        sem_t * _mapLock;
        friend void* readerProcess(void * args);
        void readCycle();
        void DoRead(Unit * pUnit);
        pthread_t _pthReader;
};


#endif
