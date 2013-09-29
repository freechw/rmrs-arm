#include "unit.h"
#include "net.h"
#include <list>
#include <vector>
#include <sys/socket.h>
#include <semaphore.h>

#include <stdio.h> //for debug

using std::vector;
using std::list;

void* senderProcess(void * args);

void Unit::addMeterId(int meterId)
{
    sem_wait(&waitingLineLock);
    waitingLine.push_back(meterId);
    sem_post(&waitingLineLock);
}

void Unit::addMeterIdQuick(int meterId)
{
    sem_wait(&waitingLineLock);
    waitingLine.push_front(meterId);
    sem_post(&waitingLineLock);
}

void Unit::loadCurrentMeters()
{
    currentLine.clear();
    sem_wait(&waitingLineLock);
    if (false == waitingLine.empty())
    {
        list<int>::size_type sz = waitingLine.size();
        for (int i = 0; (i < 4) && (i < (int)sz); i++)
        {
            int tmp;
            tmp = waitingLine.front();
            currentLine.push_back(tmp);
            waitingLine.pop_front();
        }
    }
    sem_post(&waitingLineLock);
    currentMeterNumber = 0;
    currentLineFull = false;
    currentLineCleared = false;
}

vector<int> Unit::getCurrentMeterIds()
{
    if (true == currentLine.empty())
    {
        loadCurrentMeters();
    }
    return currentLine;
}

int Unit::getCurrentMeter()
{
    return (int)currentMeterNumber;
}

void Unit::addReadyMeterData(unsigned char * dataBuf)
{
    sem_wait(&readyLineLock);
    readyLine.push_back(dataBuf);
    sem_post(&readyLineLock);
    sem_post(&uploadDataReadySignal);
    currentMeterNumber++;
}

void Unit::setNetObject(Net * net)
{
    _net = net;
}

void Unit::startSender()
{
    int status;
    status = pthread_create(&_thdSender, NULL, senderProcess, this);
    if (0 != status)
    {
        printf("sender thread creation failed!\n");
        //exit(0);
    }
}

void Unit::meterDataUploadProcess()

{
    printf("unit.cpp:meterDataUploadProcess():unit 0x%.2x start!\n", unitId);
    while(true)
    {
        sem_wait(&uploadDataReadySignal);
        sem_wait(&readyLineLock);
        if (false == readyLine.empty())
        {
            list<unsigned char *> meterData = readyLine;
            readyLine.clear();
            sem_post(&readyLineLock);
            printf("unit.cpp:meterDataUploadProcess():Unit%d, send Message\n", unitId);
            _net->sendMeterData(unitId, meterData);
        }
        else
        {
            sem_post(&readyLineLock);
            printf("unit.cpp:meterDataUploadProcess():Unit%d, Line Empty!\n", unitId);
        }
    }

    pthread_exit(0);
}

void* senderProcess(void * args)
{
    Unit * un = (Unit *)args;
    un->meterDataUploadProcess();

    return NULL;
}

void Unit::setCurrentLineFull()
{
    currentLineFull = true;
}
bool Unit::getCurrentLineFull()
{
    return currentLineFull;
}

void Unit::setCurrentLineCleared()
{
    currentLineCleared = true;
    loadCurrentMeters();
}
bool Unit::getCurrentLineCleared()
{
    return currentLineCleared;
}

void Unit::setUnitId(short id)
{
    unitId = id;
}

short Unit::getUnitId()
{
    return unitId;
}

Unit::Unit()
{
    sem_init(&readyLineLock, 0, 1);
    sem_init(&waitingLineLock, 0, 1);
    sem_init(&uploadDataReadySignal, 0, 0);

    unitId = 0;
    currentLineFull = false;
    currentLineCleared = false;

    waitingLine.clear();
    currentLine.clear();
    readyLine.clear();
}


