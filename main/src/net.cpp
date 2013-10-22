
#include "net.h"
#include <vector>
#include <list>
#include <map>

//for net
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

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
        printf("Connect Error!\n");
        sleep(30);
    }

    if (sizeof(message) != send(socketFd, message, sizeof(message), 0))
    {
        printf("HandShack Error!\n");
    }

    connected = true;

    sem_post(&sendLock);
}

void Net::reConnectServer()
{
    sem_wait(&sendLock);
    shutdown(socketFd, SHUT_RDWR);

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
    while (connect(socketFd, (struct sockaddr *)&pin, sizeof(pin)) != 0)
    {
        printf("net.cpp:reConnectServer():Reconnect Error!\n");
        sleep(30);
    }

    printf("net.cpp:reConnectServer():try hand shack\n");
    if (sizeof(message) != send(socketFd, message, sizeof(message), 0))
    {
        printf("HandShack Error!\n");
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
        sem_wait(&sendLock);
        if (length != write(socketFd, &data[0], length))
        {
            printf("net.cpp:netSend():Send Error!\n");
        }
        sem_post(&sendLock);
    }
}

Net::Net()
{
    sem_init(&sendLock, 0, 1);
    identifier = 0;
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
    /*************DEBUG************/
    printf("net.cpp:listener():listener start!\n");
    /******************************/
    while (true)
    {
        if (true == connected)
        {
            bzero(&headBuf, sizeof(headBuf));
            bool headerReceived = false;
            int recvNum = 0;

            //recv bytes until recved 13 bytes (header length is 13 bytes);
            while(false == headerReceived)
            {
                int status = recv(socketFd, &headBuf[recvNum], (13-recvNum), MSG_WAITALL);
                if (status < 0)
                {
                    printf("Recv Error\n");
                    _error_code = -1;
                    connected = false;
                    //break;
                }
                else if (0 == status)
                {
                    printf("connect lost!\n");
                    _error_code = 0;
                    connected = false;
                    reConnectServer();
                }
                else if ((status + recvNum) < 13)
                {
                    recvNum += status;
                    continue;
                }
                else if (13 == (status + recvNum))
                {
                    recvNum +=status;
                    headerReceived = true;
                }
                else
                {
                    printf("net.cpp:listener():status + recvNum is %d\n", (status + recvNum));
                }
            }

            if (false == connected)
            {
                //break;
            }

            /******DEBUG**********/
            //printf("net.cpp:listener():receved header!\n");
            /*********************/

            //check identifier and if pass, recv bytes until recived all data
            int headerId = *(int *)(&headBuf[0]);
            if (headerId == identifier)
            {
                if(0x00 == headBuf[8])
                {
                    int dataLength = *(int *)(&headBuf[9]);
                    /***********DEBUG**************/
                    //printf("net.cpp:listener():data length is %d\n", dataLength);
                    /******************************/
                    unsigned char dataBuf[dataLength];
                    bzero(&dataBuf, sizeof(dataBuf));
                    recvNum = 0;

                    bool dataReceved = false;
                    while(false == dataReceved)
                    {
                        int status = recv(socketFd, &dataBuf[recvNum], (dataLength-recvNum), MSG_WAITALL);
                        if (status < 0)
                        {
                            printf("Recv Error\n");
                            _error_code = status;
                            connected = false;
                            //break;
                        }
                        else if (0 == status)
                        {
                            printf("connect lost!\n");
                            _error_code = status;
                            connected = false;
                            //break;
                        }
                        else if ((status + recvNum) < dataLength)
                        {
                            recvNum += status;
                            continue;
                        }
                        else if (dataLength == (status + recvNum))
                        {
                            recvNum +=status;
                            dataReceved = true;
                        }
                        else
                        {
                            printf("net.cpp:listener():status + recvNum is %d\n", (status + recvNum));
                        }
                    }
                    vector<unsigned char> dataVector(dataBuf, dataBuf + (sizeof(dataBuf)/sizeof(unsigned char)));
                    unPackage(dataVector);
                }
                else
                {
                    //is not data package
                }
            }
            else
            {
                printf("net.cpp:listener():wrong identifier, recv is 0x%.2x,my is 0x%.2x\n",
                        headerId, identifier);
            }
        }
        else
        {
            connectServer();
        }
    }
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
    sem_wait(_mapLock);
    //printf("net.cpp:insertMeter():meter num is %d\n", (int)unitIds.size());
    for (int i = 0; i < (int)unitIds.size(); i++)
    {
        if ( _pUnitMap->end() != _pUnitMap->find(unitIds[i]))
        {
            //printf("net.cpp:insertMeter():insert meter id to existed unit 0x%.2x\n", unitIds[i]);
            (*_pUnitMap)[unitIds[i]]->addMeterId(meterIds[i]);
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
            (*_pUnitMap)[unitIds[i]]->addMeterId(meterIds[i]);
        }
    }
    sem_post(_mapLock);
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

        //wait for some times
        sleep(30);
    }
}

void* heartbeatProcess(void * args)
{
    Net * net = (Net *)args;
    net->heart();

    return NULL;
}


