
#include "net.h"
#include <vector>
#include <list>
#include <map>
#include <cstdlib>

//for net
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <string.h>

//for debug
#include <stdio.h>

using std::list;
using std::vector;
using std::string;
using std::map;

void* listenerProcess(void * agrs);
void* heartbeatProcess(void * args);

void Net::sendMeterData(short unitId, list<unsigned char *> data)
{
    vector<unsigned char> unitData;
    //push the upload command
    unitData.push_back(0x2D);
    //push meter number
    list<unsigned char *>::size_type dataLength = data.size();
    unitData.push_back((unsigned char)dataLength);

    //push the unitId
    unitData.push_back(unitId & 0xff);
    unitData.push_back(unitId >> 8);
    //push each byte of each meter data
    for (list<unsigned char *>::iterator iter = data.begin(); iter != data.end(); iter++)
    {
        for (int j = 0; j < METER_DATA_LENGTH; j++)
        {
            unitData.push_back((*iter)[j]);
        }
        //destory the meter data array, which new[] in Reader Class
        //move this delete[] to Record class;
        //delete[] (*iter);
    }

    //package the unitData;
    vector<unsigned char> wholeData;
    wholeData = package(unitData);
    //send wholeData
    netSend(wholeData);
}

void Net::connectServer()
{
    sem_wait(&sendLock);
    struct sockaddr_in pin;
    struct hostent *nlp_host;

    unsigned char message[] = {0x01, 0x02, 0x03, 0x04, 0xff, 0xff, 0xff, 0xff,
                      0x40, 0x00, 0x00, 0x00, 0x00};
    memcpy(message, (unsigned char *)&identifier, 4);

    while ((nlp_host = gethostbyname(_host.c_str())) == 0)
    {
        printf("Resolve Error!\n");
        sleep(5);
    }

    bzero(&pin, sizeof(pin));
    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = htonl(INADDR_ANY);
    pin.sin_addr.s_addr = ((struct in_addr *)(nlp_host->h_addr))->s_addr;
    pin.sin_port = htons(_port);

    socketFd = socket(AF_INET, SOCK_STREAM, 0);




    while (connect(socketFd, (struct sockaddr *)&pin, sizeof(pin)) == -1)
    {
      printf("net.cpp:connectServer():connect Error\n");
      sleep(30);
    }

    if (sizeof(message) != send(socketFd, message, sizeof(message), 0))
    {
        printf("HandShack Error!\n");
    }

    int x;
    x = fcntl(socketFd, F_GETFL, 0);
    if (-1 == x)
    {
      printf("net.cpp:connectServer():get fcntl error\n");
      x = 0;
    }
    if (-1 == fcntl(socketFd, F_SETFL, x | O_NONBLOCK))
    {
      printf("net.cpp:connectServer():set fcntl error\n");
    }
    connected = true;

    sem_post(&sendLock);
}

void Net::reConnectServer()
{
  printf("net.cpp:reConnectServer():sem_wait!\n");
    sem_wait(&sendLock);
    shutdown(socketFd, SHUT_RDWR);
    close(socketFd);
  printf("net.cpp:reConnectServer():wait for reConnect!\n");

    //wait 30s avoid make too many network data
    sleep(30);

    printf("net.cpp:reConnectServer():try to reconnect server!\n");
    struct sockaddr_in pin;
    struct hostent *nlp_host;

    unsigned char message[] = {0x01, 0x02, 0x03, 0x04, 0xff, 0xff, 0xff, 0xff,
                      0x40, 0x00, 0x00, 0x00, 0x00};
    memcpy(message, (unsigned char *)&identifier, 4);

    while ((nlp_host = gethostbyname(_host.c_str())) == 0)
    {
        printf("Resolve Error!\n");
        sleep(5);
    }

    bzero(&pin, sizeof(pin));
    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = htonl(INADDR_ANY);
    pin.sin_addr.s_addr = ((struct in_addr *)(nlp_host->h_addr))->s_addr;
    pin.sin_port = htons(_port);

    socketFd = socket(AF_INET, SOCK_STREAM, 0);


    printf("net.cpp:reConnectServer():try connect()\n");
    while (connect(socketFd, (struct sockaddr *)&pin, sizeof(pin)) == -1)
    {
      printf("net.cpp:reConnectServer():connect Error!\n");
        sleep(30);
    }


    printf("net.cpp:reConnectServer():try hand shack\n");
    if (sizeof(message) != send(socketFd, message, sizeof(message), 0))
    {
        printf("HandShack Error!\n");
    }
    printf("net.cpp:reConnectServer():reConnected!\n");

    int x;
    x = fcntl(socketFd, F_GETFL, 0);
    if (-1 == x)
    {
      printf("net.cpp:connectServer():get fcntl error\n");
      x = 0;
    }
    if (-1 == fcntl(socketFd, F_SETFL, x | O_NONBLOCK))
    {
      printf("net.cpp:connectServer():set fcntl error\n");
    }


    connected = true;

    sem_post(&sendLock);
}



void Net::setIpPort(string host, int port)
{
    _host = host;
    _port = port;
}

void Net::setIdentifier(int Identifier)
{
    identifier = Identifier;
}

vector<unsigned char> Net::package(vector<unsigned char> data)
{
    vector<unsigned char> rtnData;

    //push the identifier to rtnData
    unsigned char identifierBytes[4];
    memcpy(identifierBytes, (unsigned char *)&identifier, sizeof(identifierBytes));
    for (int i = 0; i < (int)sizeof(identifierBytes); i++)
    {
        rtnData.push_back(identifierBytes[i]);
    }

    //push chucksum to rtnData
    for (int i = 0; i < 4; i++)
    {
        rtnData.push_back(0xff);
    }

    //push data command code to rtnData
    rtnData.push_back(0x00);

    //push data length to rtnData
    int dataLength = (int)data.size();
    unsigned char dataLengthBytes[4];
    memcpy(dataLengthBytes, (unsigned char *)&dataLength, 4);
    for (int i = 0; i < 4; i++)
    {
        rtnData.push_back(dataLengthBytes[i]);
    }

    //push data to rtnData
    rtnData.insert(rtnData.end(), data.begin(), data.end());

    return rtnData;
}

void Net::start()
{
    connectServer();


    int status;
    status = pthread_create(&_thdListener, NULL, listenerProcess, this);
    if (0 != status)
    {
        printf("net.cpp:start():listener thread creation failed!\n");
        //exit(0);
    }

    status = pthread_create(&_thdHeart, NULL, heartbeatProcess, this);
    if (0 != status)
    {
        printf("net.cpp:start():heart process creation fail!\n");
    }

}

void Net::netSend(vector<unsigned char> data)
{
    if (false == data.empty())
    {
        int length = (int)data.size();
        int sendLength = 0;
        int tmpLength = 0;
        sem_wait(&sendLock);
        while ( length > sendLength)
        {
          tmpLength = write(socketFd, &data[sendLength], length - sendLength);
          if (0 < tmpLength)
          {
            sendLength += tmpLength;
          }
          else if ((EAGAIN == errno) || (EWOULDBLOCK == errno) || (EINTR == errno))
          {
            //
          }
          else
          {
            //send is wrong , just break to return function( if server can recv heart beat, socket will be reset)
            break;
          }
        }

        sem_post(&sendLock);
    }
}

Net::Net()
{
    sem_init(&sendLock, 0, 1);
    if (0 == sem_init(&heartTimesLock_, 0, 1))
    {
      printf("init heart lock ok!\n");
    }
    identifier = 0;
    heartTimes_ = 0;
    lastTime_ = 0;
    heartCount_ = 0;
    _port = 8000;
    _host = "127.0.0.1";
    connected = false;
}

void* listenerProcess(void * args)
{
    Net * net = (Net *)args;
    while(true)
    {
        try
        {
            net->listener();
        }
        catch(int * pCode)
        {
            printf("net.cpp:listenerProcess():catch excepction! Code is %d\n", *pCode);
            net->reConnectServer();
        }
    }


    return NULL;
}

void Net::listener()
{
    unsigned char headBuf[13];
    time_t tmpTime;
    time(&tmpTime);
    lastTime_ = (int)tmpTime;
    /*************DEBUG************/
    printf("net.cpp:listener():listener start!\n");
    /******************************/
    while (true)
    {
      try
      {
        bzero(&headBuf, 13);
        this->ReadFromTcp(13, headBuf);
        this->AnalyzeHeader(headBuf);
      }
      catch (unsigned int * errorCode)
      {
        unsigned int code = *errorCode;
        printf("net.cpp;listener():Excpetion! %d\n", code);
        this->reConnectServer();
      }
    }
}

void Net::AnalyzeHeader(unsigned char headBuf[])
{
  int headerId = *(int *)(&headBuf[0]);
  unsigned char tmpType = headBuf[8];
  int tmpLength = *(int *)(&headBuf[9]);

  //check identifier
  if(headerId != this->identifier)
  {
    //TODO
    unsigned int errorCode = 0x7d;
    throw &errorCode;
  }

  switch (tmpType)
  {
    case 0x00:
      sem_wait(&heartTimesLock_);
      this->heartTimes_ = 0;
      sem_post(&heartTimesLock_);
      this->RecvDataPackage(tmpLength);
      break;
    case 0x80:
      sem_wait(&heartTimesLock_);
      this->heartTimes_ = 0;
      sem_post(&heartTimesLock_);
      break;
    default:
      //TODO
      unsigned int errorCode = 1;
      throw &errorCode;
      break;
  }
}

void Net::RecvDataPackage(int dataLength)
{
  unsigned char dataBuf[dataLength];
  this->ReadFromTcp(dataLength, dataBuf);
  vector<unsigned char> dataVector(dataBuf, dataBuf + (sizeof(dataBuf)/sizeof(unsigned char)));
  this->unPackage(dataVector);
}


void Net::ReadFromTcp(int length, unsigned char recvBuf[])
{
  int recevidLength = 0;
  int tmpLength = 0;

  while (length > recevidLength)
  {
    tmpLength =  read(socketFd, &recvBuf[recevidLength], (length-recevidLength));
    if (0 < tmpLength)
    {
      recevidLength += tmpLength;
    }
    else if (0 == tmpLength)
    {
      unsigned int errorCode = 0xc1;
      throw &errorCode;
    }
    else if ((EINTR == errno) || (EWOULDBLOCK == errno) || (EAGAIN == errno))
    {
      //continue;
    }
    else
    {
      unsigned int errorCode = 0xc1;
      throw &errorCode;
    }

//    printf("net.cpp:RFT():TO CHB()\n");

    CheckHeartBeat();
  }
}

void Net::CheckHeartBeat()
{
//  printf("net.cpp:CHB() Start!\n");
  int tmpTimes;
  int tmpCount;
  time_t currentTime;
  time(&currentTime);
  int tmpSpan = lastTime_- currentTime;
//  printf("lastTime is %d, currentTime is %d\n", lastTime_, (int)currentTime);
  tmpSpan = abs(tmpSpan);
//  printf("net.cpp:CHB():tmpSpan is %d\n", tmpSpan);
  if ((int)3 < tmpSpan)
  {
    sem_wait(&heartTimesLock_);
//    printf("net.cpp:CHB() Enter\n");
    heartTimes_++;
    tmpTimes = heartTimes_;
    tmpCount = heartCount_;
    sem_post(&heartTimesLock_);
//    printf("net.cpp:CHB() Exit\n");

    //wait for 100 * 3 seconds to recv heart beat package
    if (100 < tmpTimes)
    {
      printf("net.cpp:CHB():heart beat lost! %d\n", tmpCount);
      sem_wait(&heartTimesLock_);
      heartTimes_ = 0;
      sem_post(&heartTimesLock_);
      unsigned int errorCode = 2;
      throw &errorCode;
    }

    time(&currentTime);
    lastTime_ = (int)currentTime;
  }
//  printf("net.cpp:CHB() End!\n");
}




void Net::unPackage(vector<unsigned char> data)
{
    if (false == data.empty())
    {
        //ack this command
        vector<unsigned char>ackData(data);
        ackData[0] = ackData[0] -3;
        netSend(package(ackData));

        if ((unsigned char)data.size() == ((data[1] * 6) + 2))
        {
            //7d is the normal read meter command
            if (0x7d == data[0])
            {
                unsigned int meterNum = data[1];
                printf("net.cpp:unPackage():meter num is %d\n", meterNum);
                vector<short> unitIds;
                vector<int> meterIds;
                for (int i = 0; i < (int)meterNum; i++)
                {
                    unsigned int tmpMeterBase = 2 + (i * 6);
                    unitIds.push_back(*(short *)(&data[tmpMeterBase]));
                    meterIds.push_back(*(int *)(&data[2 + tmpMeterBase]));

                    /**********DEBUG***************/
                    //printf("net.cpp:unPackage():unitId is 0x%.2x, meterId is 0x%.2x\n",
                    //        unitIds.back(), meterIds.back());
                }
                insertMeter(unitIds, meterIds);
            }
            //0d is the quick read meter command
            else if (0x0d== data[0])
            {
                unsigned int meterNum = data[1];
                printf("net.cpp:unPackage():quick meter num is %d\n", meterNum);
                vector<short> unitIds;
                vector<int> meterIds;
                for (int i = 0; i < (int)meterNum; i++)
                {
                    unsigned int tmpMeterBase = 2 + (i * 6);
                    unitIds.push_back(*(short *)(&data[tmpMeterBase]));
                    meterIds.push_back(*(int *)(&data[2 + tmpMeterBase]));

                    /**********DEBUG***************/
                    //printf("net.cpp:unPackage():unitId is 0x%.2x, meterId is 0x%.2x\n",
                    //        unitIds.back(), meterIds.back());
                }
                insertMeterQuick(unitIds, meterIds);
            }

        }
    }
}

void Net::setUnitMap(map<short, Unit *> * pUnitMap, sem_t * mapLock)
{
    _pUnitMap = pUnitMap;
    _mapLock = mapLock;
}

void Net::setRecordPointer(Record * pRecord)
{
    _pRecord = pRecord;
}

void Net::insertMeter(vector<short> unitIds, vector<int> meterIds)
{
    printf("net.cp:insertMeter():Enter!\n");
    sem_wait(_mapLock);
    printf("net.cpp:insertMeter():meter num is %d\n", (int)unitIds.size());
    for (int i = 0; i < (int)unitIds.size(); i++)
    {
        if ( _pUnitMap->end() != _pUnitMap->find(unitIds[i]))
        {
            printf("net.cpp:insertMeter():insert meter id to existed unit 0x%.2x\n", unitIds[i]);
            (*_pUnitMap)[unitIds[i]]->addMeterId(meterIds[i]);
        }
        else
        {
            printf("net.cpp:insertMeter():insert meter id to new unit 0x%.2x\n", unitIds[i]);
            Unit * pTmpUnit = new Unit();
            pTmpUnit->setUnitId(unitIds[i]);
            pTmpUnit->setNetObject(this);
            pTmpUnit->setRecordObject(_pRecord);
            pTmpUnit->startSender();
            _pUnitMap->insert(map<short, Unit *>::value_type(unitIds[i], pTmpUnit));
            (*_pUnitMap)[unitIds[i]]->addMeterId(meterIds[i]);
        }
    }
    sem_post(_mapLock);
    printf("net.cpp:insertMeter():Insert End\n");
}

void Net::insertMeterQuick(vector<short> unitIds, vector<int> meterIds)
{
    sem_wait(_mapLock);
    //printf("net.cpp:insertMeter():meter num is %d\n", (int)unitIds.size());
    for (int i = 0; i < (int)unitIds.size(); i++)
    {
        if ( _pUnitMap->end() != _pUnitMap->find(unitIds[i]))
        {
            //printf("net.cpp:insertMeter():insert meter id to existed unit 0x%.2x\n", unitIds[i]);
            (*_pUnitMap)[unitIds[i]]->addMeterIdQuick(meterIds[i]);
        }
        else
        {
            //printf("net.cpp:insertMeter():insert meter id to new unit 0x%.2x\n", unitIds[i]);
            Unit * pTmpUnit = new Unit();
            pTmpUnit->setUnitId(unitIds[i]);
            pTmpUnit->setNetObject(this);
            pTmpUnit->setRecordObject(_pRecord);
            pTmpUnit->startSender();
            _pUnitMap->insert(map<short, Unit *>::value_type(unitIds[i], pTmpUnit));
            (*_pUnitMap)[unitIds[i]]->addMeterIdQuick(meterIds[i]);
        }
    }
    sem_post(_mapLock);
}

void Net::heart()
{
    while (true)
    {
      /***********DEBUG************/
      printf("net.cpp:heart():try send heartBeat Package\n");
      /****************************/

        vector<unsigned char> heartbeatPackage;

        //push the identifier to heartbeatPackage
        unsigned char identifierBytes[4];
        memcpy(identifierBytes, (unsigned char *)&identifier, sizeof(identifierBytes));
        for (int i = 0; i < (int)sizeof(identifierBytes); i++)
        {
            heartbeatPackage.push_back(identifierBytes[i]);
        }

        //push chucksum to heartbeatPackage
        for (int i = 0; i < 4; i++)
        {
            heartbeatPackage.push_back(0xff);
        }

        //push heart command code to heartbeatPackage
        heartbeatPackage.push_back(0x80);

        //push data length(0x00000000) to heartbeatPackage
        for (int i = 0; i < 4; i++)
        {
            heartbeatPackage.push_back(0x00);
        }

        netSend(heartbeatPackage);

        /*******DEBUG************/
        printf("net.cpp:heart():send end\n");
        /************************/

        //wait for some seconds
        sleep(10);
    }
}

void* heartbeatProcess(void * args)
{
    Net * net = (Net *)args;
    net->heart();

    return NULL;
}


