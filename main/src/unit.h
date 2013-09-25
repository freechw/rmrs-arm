#ifndef __CLASS_UNIT__
#define __CLASS_UNIT__

#include <vector>
#include <list>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>

class Unit
{
    public:
        void addMeterId(int meterId);
        void addMeterIdQuick(int meterId);
        std::vector<int> getCurrentMeterIds();
        int getCurrentMeter();
        void addReadyMeterData(unsigned char dataBuf[]);
        void setTcpObject(int* tcp);//TO do, change int type to my own tcp class
        void startSender();
        void setCurrentLineFull();
        bool getCurrentLineFull();
        void setCurrentLineCleared();
        bool getCurrentLineCleared();
        void setUnitId(short id);
        short getUnitId();

        Unit();
    private:
        std::list<int> waitingLine;
        unsigned int currentMeterNumber;
        std::vector<int> currentLine;
        std::list<unsigned char *> readyLine;
        int* _tcp;
        void loadCurrentMeters();

        pthread_t _thdSender;
        friend void* senderProcess(void * args);
        void meterDataUploadProcess();
        bool currentLineFull;
        bool currentLineCleared;
        short unitId;

        sem_t readyLineLock;
        sem_t waitingLineLock;
        sem_t uploadDataReadySignal;

};


#endif

