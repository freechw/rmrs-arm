#include "reader.h"
#include <string.h>

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
        if (true == _pSi4432->isReceived())
        {
            vector<unsigned char> rdBackData = _pSi4432->fifoRead();
            if (isCurrent(rdBackData, meterIds))
            {
                if (rdBackData[4] == rdBackData[5])//Collect ready to upload
                {
                    int tmpMeterNumber = pUnit->getCurrentMeter();
                    sendUploadCommand(pUnit->getUnitId(), tmpMeterNumber);
                    if (true == _pSi4432->isReceived())
                    {
                        vector<unsigned char> uploadData = _pSi4432->fifoRead();

                        unsigned char * pData = new unsigned char[38];//TO do
                        memcpy(pData, &uploadData[3], 38);
                        printf("reader.cpp:DoRead():add ready meter data!\n");
                        pUnit->addReadyMeterData(pData);
                        int tmpMeterId = meterIds[tmpMeterNumber];
                        if (tmpMeterId == meterIds.back())
                        {
                            pUnit->setCurrentLineFull();
                        }
                    }
                    else
                    {
                        //add meter data to unit with last byte is 0xa1(means unit lost)
                        unsigned char * pData = new unsigned char[38];
                        pData[37] = 0xa1;
                        int tmpMeterId = meterIds[tmpMeterNumber];
                        *(int *)(&pData[0]) = tmpMeterId;
                        pUnit->addReadyMeterData(pData);
                        if (tmpMeterId == meterIds.back())
                        {
                            pUnit->setCurrentLineFull();
                            pUnit->setCurrentLineCleared();
                        }
                    }
                }
            }
            else
            {
                //clear last meter data
                sendClearCommand(pUnit->getUnitId());
                _pSi4432->isReceived();
            }
        }
        else
        {
            int tmpMeterNumber = pUnit->getCurrentMeter();
            int tmpMeterId = meterIds[tmpMeterNumber];
            unsigned char * pData = new unsigned char[38];
            pData[37] = 0xa1;
            *(int *)(&pData[0]) = tmpMeterId;
            pUnit->addReadyMeterData(pData);
            if (tmpMeterId == meterIds.back())
            {
                pUnit->setCurrentLineFull();
                pUnit->setCurrentLineCleared();
            }
        }
    }
    else
    {
        if (false == pUnit->getCurrentLineCleared())
        {
            sendClearCommand(pUnit->getUnitId());
            if (true == _pSi4432->isReceived())
            {
                vector<unsigned char> clData = _pSi4432->fifoRead();
                if ( 0xc1 == clData[3])
                {
                    pUnit->setCurrentLineCleared();
                }
            }
            else
            {
                //tmp lost unit
                if (2 < pUnit->getClearNum())
                {
                    pUnit->setCurrentLineCleared();
                }
                else
                {
                    pUnit->plusClearNum();
                    printf("reader.cpp:DoRead():clear num is %d\n", pUnit->getClearNum());
                }
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
    printf("reader.cpp:sendReadCommand():send rd command!\n");
    printf("unit id is: 0x%.2x\n", unitId);
    printf("meter id list:");
    for (int i = 0; i < (int)meterIds.size(); i++)
    {
        printf(" 0x%.2x,", meterIds[i]);
    }
    printf("\n");

    vector<unsigned char> readCommandBuf(7+ (4 * meterIds.size()));
    //add unitId
    memcpy(&readCommandBuf[0], (unsigned char *)(&unitId), 2);
    //add data length
    readCommandBuf[2] = 4 + (unsigned char)(4 * meterIds.size());
    //add read command
    readCommandBuf[3] = 0x7d;
    //add total number
    readCommandBuf[4] = (unsigned char)(meterIds.size());
    //clear read number and trans number
    readCommandBuf[5] = 0x00;
    readCommandBuf[6] = 0x00;
    //add meterIds
    memcpy(&readCommandBuf[7], (unsigned char *)(&meterIds[0]),
            (4 * meterIds.size()));
    //add stop hex
    readCommandBuf.push_back(0x57);

    _pSi4432->fifoSend(readCommandBuf);

}

void Reader::sendUploadCommand(short unitId, int meterId)
{
    //To do......
    printf("reader.cpp:sendUploadCommand():send up command!\n");
    printf("unit id is: 0x%.2x\n", unitId);
    printf("meter id is: 0x%.2x\n", meterId);

    //TO Do ( change 4 * 4 to 4 * currentMeterIds.size() or 0)
    vector<unsigned char> uploadCommandBuf(7+ (4 * 4));
    //add unitId
    memcpy(&uploadCommandBuf[0], (unsigned char *)(&unitId), 2);
    //add data legnth
    uploadCommandBuf[2] = 20;
    //add upload command
    uploadCommandBuf[3] = 0x2d;
    //add total number
    //TO DO ( change 4 to meterId.size() or 0);
    uploadCommandBuf[4] = (unsigned char)(4);
    //clear read number
    uploadCommandBuf[5] = 0x00;
    //set trans number
    //TO DO ( change meterId to current meter's sequence number in meterIds )
    uploadCommandBuf[6] = 1 + meterId;//in collector this nunber will -1;
    //add stop hex
    uploadCommandBuf.push_back(0x57);

    _pSi4432->fifoSend(uploadCommandBuf);

}

void Reader::sendClearCommand(short unitId)
{
    printf("reader.cpp:sendClearCommand():send cl command!\n");
    printf("unit id is: 0x%.2x\n", unitId);

    //TO Do ( change 4 * 4 to 4 * currentMeterIds.size() or 0)
    vector<unsigned char> clearCommandBuf(7+ (4 * 4));
    //add unitId
    memcpy(&clearCommandBuf[0], (unsigned char *)(&unitId), 2);
    //add data length
    clearCommandBuf[2] = 20;
    //add clear command
    clearCommandBuf[3] = 0xc1;
    //add total number
    //TO DO ( change 4 to meterId.size() or 0);
    clearCommandBuf[4] = (unsigned char)(4);
    //clear read number and trans number
    clearCommandBuf[5] = 0x00;
    clearCommandBuf[6] = 0x00;
    //add stop hex
    clearCommandBuf.push_back(0x57);

    _pSi4432->fifoSend(clearCommandBuf);

}

void Reader::setSi4432(Si4432 * pSi4432)
{
    _pSi4432 = pSi4432;
}

//check the rd command back buf is suit for current meterids
bool Reader::isCurrent(vector<unsigned char> rdBackData, vector<int> currentMeters)
{
    if (rdBackData.size() != (8 + 4 * currentMeters.size()))
    {
        return false;
    }

    for (int i = 0; i < (int)currentMeters.size(); i++)
    {
        if (*(int *)(&rdBackData[7 + i * 4]) != currentMeters[i])
        {
            return false;
        }
    }

    return true;
}


