#ifndef __CLASS_NET__
#define __CLASS_NET__

#include <list>
#include <vector>
#include <map>
#include <string>
#include <semaphore.h>
#include <pthread.h>
#include "unit.h"

#define METER_DATA_LENGTH 38

class Net
{
    public:
        void sendMeterData(short unitId, std::list<unsigned char *> data);
        void start();
        void setIpPort(std::string host, int port);
        void setIdentifier(int Identifier);
        void setUnitMap(std::map<short, Unit *> * pUnitMap, sem_t * mapLock);
        Net();
    private:
        int socketFd;
        int identifier;
        int _port;
        sem_t sendLock;
        std::string _host;
        std::map<short, Unit *> * _pUnitMap;
        sem_t * _mapLock;
        void connectServer();
        void reConnectServer();
        void netSend(std::vector<unsigned char> data);
        std::vector<unsigned char> package(std::vector<unsigned char> data);
        void listener();
        void unPackage(std::vector<unsigned char> data);
        void ack(std::vector<unsigned char> data);
        pthread_t _thdListener;
        friend void* listenerProcess(void *);
        void insertMeter(std::vector<short> unitIds, std::vector<int> meterIds);
};

#endif
