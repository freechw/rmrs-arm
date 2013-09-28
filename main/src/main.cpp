#include "unit.h"
#include "net.h"
#include "reader.h"
#include "spi.h"
#include "si4432.h"
#include <stdio.h>
#include <vector>
#include <list>
#include <string>
#include <map>


using std::list;
using std::string;
using std::vector;
using std::map;

int main()
{
//    Unit unit1;
//    unit1.setUnitId(0x0101);
//    for (int i = 1; i < 8; i++)
//    {
//        unit1.addMeterId(i);
//    }
//
//    unit1.setCurrentLineCleared();
//
//    vector<int> tmpLine;
//    tmpLine = unit1.getCurrentMeterIds();
////    int f = tmpLine.front();
////    printf(" %d\n", f);
//    vector<int>::const_iterator iter = tmpLine.begin();
//    while (tmpLine.end() != iter)
//    {
//        printf(" %d", *iter);
//        iter++;
//    }
//    printf("\n");
//
//    unit1.startSender();

//    map<short, Unit *> mainUnitMap;
//
//    sem_t mapLock;
//    sem_init(&mapLock, 0, 1);
//
//
//
//    Net netCenter;
//    netCenter.setIdentifier(0x11223344);
//    netCenter.setIpPort("192.168.1.158", 8000);
//    netCenter.setUnitMap(&mainUnitMap, &mapLock);
//    netCenter.start();
//
//    Reader mainReader;
//    mainReader.setUnitMap(&mainUnitMap, &mapLock);
//    mainReader.start();

//    unit1.setNetObject(&netCenter);

//    for (int i = 0; i < 4; i++)
//    {
//        unsigned char *pchar = new unsigned char[38];
//        for (int j = 0; j < 38; j++)
//        {
//            pchar[j] = j;
//        }
//        unit1.addReadyMeterData(pchar);
//    }

    Spi spi;
    spi.setDevices("/dev/spidev1.0", "/dev/my_led");
    spi.chipOpen();
//    spi.chipWrite(0x12, 0x13);
//    printf("main():0x12 register is 0x%.2x\n", spi.chipRead(0x12));
    Si4432 si4432;
    si4432.setSpi(&spi);
    si4432.reset();
    si4432.init();
    si4432.setIdleMode();

    vector<unsigned char> testData(7);
    testData[0] = 0x12;
    testData[1] = 0x7d;
    testData[6] = 0x16;

    si4432.fifoSend(testData);
    if (true == si4432.isReceived())
    {
        vector<unsigned char> recvData;
        recvData = si4432.fifoRead();
        printf("main.cpp:main():recv data is:");
        for (int i = 0; i < (int)recvData.size(); i++)
        {
            printf(" 0x%.2x ", recvData[i]);
        }
        printf("\n");
    }

    printf("main.cpp:main():set idle mode!\n");
    si4432.setIdleMode();

    while(true);

    return 0;
}

