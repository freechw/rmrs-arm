#ifndef __CLASS_RECORD__
#define __CLASS_RECORD__

#include <list>
#include <string>
#include <semaphore.h>
#include <fstream>


class Record
{
    public:
        void setFile(std::string filepath);
        void recordMeterData(std::list<unsigned char *> meterData);
        Record();
        void closeFile();
    private:
        sem_t fileLock;
        std::ofstream recordFile;
        int recordFileFd;
};


#endif
