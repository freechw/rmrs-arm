#ifndef __CLASS_NET__
#define __CLASS_NET__

#include <list>
#include <vector>
#include <string>
#include <semaphore.h>
#include <pthread.h>

#define METER_DATA_LENGTH 38

class Net
{
    public:
        void sendMeterData(short unitId, std::list<unsigned char *> data);
        void start();
        void setIpPort(std::string host, int port);
        void setIdentifier(int Identifier);
        Net();
    private:
        int socketFd;
        int identifier;
        int _port;
        sem_t sendLock;
        std::string _host;
        void connectServer();
        void reConnectServer();
        void netSend(std::vector<unsigned char> data);
        std::vector<unsigned char> package(std::vector<unsigned char> data);
        void listener();
        void unPackage(std::vector<unsigned char> data);
        void ack(std::vector<unsigned char> data);
        pthread_t _thdListener;
        friend void* listenerProcess(void *);
};

#endif
