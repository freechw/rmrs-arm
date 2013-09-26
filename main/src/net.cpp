#include "net.h"
#include <vector>
#include <list>

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

void Net::sendMeterData(short unitId, list<unsigned char *> data)
{
    vector<unsigned char> unitData;
    //push the unitId
    unitData.push_back(unitId >> 8);
    unitData.push_back(unitId & 0xff);
    //push meter number
    list<unsigned char *>::size_type dataLength = data.size();
    unitData.push_back((unsigned char)dataLength);
    //push each byte of each meter data
    list<unsigned char *>::iterator iter;data.begin();
    for (list<unsigned char *>::iterator iter = data.begin(); iter != data.end(); iter++)
    {
        for (int j = 0; j < METER_DATA_LENGTH; j++)
        {
            unitData.push_back((*iter)[j]);
        }
        //destory the meter data array, which new[] in Reader Class
        delete[] (*iter);
    }

    //package the unitData;
    vector<unsigned char> wholeData;
    wholeData = package(unitData);
    //send wholeData
    netSend(wholeData);
}

void Net::connectServer()
{
    struct sockaddr_in pin;
    struct hostent *nlp_host;

    unsigned char message[] = {0x01, 0x02, 0x03, 0x04, 0xff, 0xff, 0xff, 0xff,
                      0x40, 0x00, 0x00, 0x00, 0x00};
    memcpy(message, (unsigned char *)&identifier, 4);

    while ((nlp_host = gethostbyname(_host.c_str())) == 0)
    {
        printf("Resolve Error!\n");
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
    }

    while (sizeof(message) != send(socketFd, message, sizeof(message), 0))
    {
        printf("HandShack Error!\n");
    }
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
    for (int i = 0; i < sizeof(identifierBytes); i++)
    {
        rtnData.push_back(identifierBytes[i]);
    }

    //push chucksum to rtnData
    for (int i = 0; i < 4; i++)
    {
        rtnData.push_back(0xff);
    }

    //push data command code to rtnData
    rtnData.push_back(0x80);

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
}

void Net::netSend(vector<unsigned char> data)
{
    if (false == data.empty())
    {
        int length = (int)data.size();
        sem_wait(&sendLock);
        if (length != send(socketFd, &data[0], length, 0))
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
}

void listenerProcess(void * args)
{
    Net * net = (Net *)args;
    net->listener();
}

void Net::listener()
{
    unsigned char headBuf[13];
    while (true)
    {
        bzero(&headBuf, sizeof(headBuf));
        bool headerReceived = false;
        int recvNum = 0;

        //recv bytes until recved 13 bytes (header length is 13 bytes);
        while(false == headerReceived)
        {
            int status = recv(socketFd, &headBuf[recvNum], (13-recvNum), 0);
            if (status < 0)
            {
                printf("Recv Error\n");
                continue;
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

        /******DEBUG**********/
    }
}

