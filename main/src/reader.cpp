#include "reader.h"

using std::vector;
using std::list;
using std::map;

void* readerProcess(void * args);

void Reader::setUnitMap(map<short, Unit *> * pUnitMap, sem_t * mapLock)
{
    _pUnitMap = pUnitMap;
    _mapLock = mapLock;
}

void Reader::readCycle()
{
    map<short, Unit *> tmpUnitMap;
    while(true)
    {
        tmpUnitMap.clear();
        sem_wait(_mapLock);
        tmpUnitMap.insert(_pUnitMap->begin(), _pUnitMap->end());
        sem_post(_mapLock);
        for (map<short, Unit *>::const_iterator iter = tmpUnitMap.begin();
             iter != tmpUnitMap.end(); iter++)
        {
            Unit * pTmpUnit = (iter->second);
            DoRead(pTmpUnit);
        }
    }
}

void Reader::DoRead(Unit * pUnit)
{
    if (false == pUnit->getCurrentLineFull())
    {
        vector<int> meterIds = pUnit->getCurrentMeterIds();
        if (true == meterIds.empty())
        {
            return;
        }
        sendReadCommand(pUnit->getUnitId(), meterIds);
        //while(false == interrutp && false == timer);
        if (/* ture == interrutp */ true)
        {
            //recvLowerMessage();
            if (/* upload == true */ true)
            {
                int tmpMeterId = pUnit->getCurrentMeter();
                sendUploadCommand(pUnit->getUnitId(), tmpMeterId);
                //while(false == interrupt && false == timer);
                if (/* ture == interrupt */ true)
                {
                    //recvLowerMessage();
                    unsigned char * pData = new unsigned char[38];//TO do
                    *(int *)(&pData[0]) = tmpMeterId;
                    pData[37] = 0x00;
                    printf("reader.cpp:DoRead():add ready meter data!\n");
                    pUnit->addReadyMeterData(pData);
                    if ( /* tmpMeterId == meterIds.back() */ true)
                    {
                        pUnit->setCurrentLineFull();
                    }
                }
                else
                {
                    //add meter data to unit with last byte is 0x27(means unit lost)
                    unsigned char * pData = new unsigned char[38];
                    pData[37] = 0x27;
                    *(int *)(&pData[0]) = tmpMeterId;
                    pUnit->addReadyMeterData(pData);
                }
            }
        }
        else
        {
            int tmpMeterId = pUnit->getCurrentMeter();
            unsigned char * pData = new unsigned char[38];
            pData[37] = 0x27;
            *(int *)(&pData[0]) = tmpMeterId;
            pUnit->addReadyMeterData(pData);
        }
    }
    else
    {
        if (false == pUnit->getCurrentLineCleared())
        {
            sendClearCommand(pUnit->getUnitId());
            //while(false == interrupt && false == timer)
            if (/* true == interrupt */ true)
            {
                pUnit->setCurrentLineCleared();
            }
            else
            {
                //tmp lost unit
            }
        }
    }
}

void Reader::start()
{
    int status;
    status = pthread_create(&_pthReader, NULL, readerProcess, this);
    if (0 != status)
    {
        printf("sender thread creation failed!\n");
        //exit(0);
    }
}

void* readerProcess(void * args)
{
    Reader * rd = (Reader *)args;
    rd->readCycle();

    return NULL;
}

void Reader::sendReadCommand(short unitId, vector<int> meterIds)
{
    //TO do......
    printf("reader.cpp:sendReadCommand():send rd command!\n");
    printf("unit id is: 0x%.2x\n", unitId);
    printf("meter id list:");
    for (int i = 0; i < (int)meterIds.size(); i++)
    {
        printf(" 0x%.2x,", meterIds[i]);
    }
    printf("\n");
}

void Reader::sendUploadCommand(short unitId, int meterId)
{
    //To do......
    printf("reader.cpp:sendUploadCommand():send up command!\n");
    printf("unit id is: 0x%.2x\n", unitId);
    printf("meter id is: 0x%.2x\n", meterId);
}

void Reader::sendClearCommand(short unitId)
{
    printf("reader.cpp:sendClearCommand():send cl command!\n");
    printf("unit id is: 0x%.2x\n", unitId);
}


