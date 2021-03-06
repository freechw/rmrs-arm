#ifndef RMRS_MAIN_SRC_NET_H_
#define RMRS_MAIN_SRC_NET_H_


#include <list>
#include <vector>
#include <map>
#include <string>
#include <semaphore.h>
#include <pthread.h>
#include "unit.h"
#include "reader.h"
#include "time.h"

#define METER_DATA_LENGTH 38

using namespace std;



class Net
{
  public:
    void sendMeterData(short unitId, std::list<unsigned char *> data);
    void start();
    void setIpPort(std::string host, int port);
    void setIdentifier(int Identifier);
    void setUnitMap(std::map<short, Unit *> * pUnitMap, sem_t * mapLock);
    void setRecordPointer(Record * pRecord);
    Net();
  private:
    int socketFd;
    int identifier;
    int _port;
    sem_t sendLock;
    sem_t heartTimesLock_;
    int heartTimes_;
    std::string _host;
    std::map<short, Unit *> * _pUnitMap;
    Record * _pRecord;
    sem_t * _mapLock;
    void connectServer();
    void reConnectServer();
    void netSend(std::vector<unsigned char> data);
    std::vector<unsigned char> package(std::vector<unsigned char> data);
    void listener();
    void heart();
    void unPackage(std::vector<unsigned char> data);
    void ack(std::vector<unsigned char> data);
    pthread_t _thdListener;
    pthread_t _thdHeart;
    friend void* listenerProcess(void *);
    friend void* heartbeatProcess(void *);
    void insertMeter(std::vector<short> unitIds, std::vector<int> meterIds);
    void insertMeterQuick(std::vector<short> unitIds, std::vector<int> meterIds);
    int _error_code;
    bool connected;
    void ReadFromTcp(int length, unsigned char recvBuf[]);
    void AnalyzeHeader(unsigned char headBuf[]);
    void RecvDataPackage(int length);
    void CheckHeartBeat();
    int lastTime_;
    int heartCount_;

};



#endif
