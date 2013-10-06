#ifndef __CLASS_UNIT__
#define __CLASS_UNIT__

#include <vector>
#include <list>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
//#include "net.h"
class Net;
#include "record.h"

class Unit
{
    public:
        void addMeterId(int meterId);
        void addMeterIdQuick(int meterId);
        std::vector<int> getCurrentMeterIds();
        int getCurrentMeter();
        void addReadyMeterData(unsigned char dataBuf[]);
        void setNetObject(Net * net);
        void setRecordObject(Record * record);
        void startSender();
        void setCurrentLineFull();
        bool getCurrentLineFull();
        void setCurrentLineCleared();
        bool getCurrentLineCleared();
        void setUnitId(short id);
        short getUnitId();
        void plusClearNum();
        unsigned char getClearNum();

        Unit();
    private:
        std::list<int> waitingLine;
        unsigned int currentMeterNumber;
        std::vector<int> currentLine;
        std::list<unsigned char *> readyLine;
        Net * _net;
        Record * _record;
        void loadCurrentMeters();

        pthread_t _thdSender;
        friend void* senderProcess(void * args);
        void meterDataUploadProcess();
        bool currentLineFull;
        bool currentLineCleared;
        short unitId;
        unsigned char _clearNum;

        sem_t readyLineLock;
        sem_t waitingLineLock;
        sem_t uploadDataReadySignal;

};


#endif

