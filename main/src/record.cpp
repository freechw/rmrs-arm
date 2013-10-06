#include "record.h"
#include <vector>
#include <list>
#include <fstream>
#include <string>
#include <semaphore.h>
#include <time.h>
#include <string.h>

using std::vector;
using std::string;
using std::list;



#define METER_DATA_LENGTH 38

void Record::setFile(string filepath)
{
    recordFile.open(filepath.c_str(), std::ios::app);
}

void Record::closeFile()
{
    recordFile.close();
}

void Record::recordMeterData(list<unsigned char *> meterData)
{
    if (true == meterData.empty())
    {
        return;
    }

    list< vector<unsigned char> >recordDatalist;

    for (list<unsigned char *>::iterator iter = meterData.begin(); iter != meterData.end(); iter++)
    {
        vector<unsigned char>singleData(4);
        //push current time
        time_t rawtime;
        time(&rawtime);
        memcpy(&singleData[0], (unsigned char *)(&rawtime), 4);

        //push meter data
        for (int j = 0; j < METER_DATA_LENGTH; j++)
        {
            singleData.push_back((*iter)[j]);
        }
        delete[] *(iter);

        //push EE to fill the data to 48 bytes;
        for (int i = 0; i < 6; i++)
        {
            singleData.push_back(0xEF);
        }

        //add this meter data with time stamp to data list;
        recordDatalist.push_back(singleData);
    }

    sem_wait(&fileLock);
    if (true == recordFile.is_open())
    {
        for (list< vector<unsigned char> >::iterator iter = recordDatalist.begin();
                iter != recordDatalist.end();
                iter++)
        {
            recordFile.write((char *)(&(*iter)[0]), 48);
        }
    }
    recordFile.flush();
    printf("record.cpp:recordMeterData():flush to disk file\n");
    sem_post(&fileLock);
}

Record::Record()
{
    sem_init(&fileLock, 0, 1);
}


