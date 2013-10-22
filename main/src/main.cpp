#include "unit.h"
#include "net.h"
#include "reader.h"
#include "spi.h"
#include "si4432.h"
#include "record.h"
#include <stdio.h>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <cmath>
#include <cstdlib>


using std::list;
using std::string;
using std::vector;
using std::map;

int main(int argc, char* argv[])
{
    unsigned int identifier = 0;
    string serverIp;
    int serverPort;
    short syncword;
    if (argc != 5)
    {
        printf("error args!\n");
        identifier = 0x11223344;
        serverIp = "192.168.1.158";
        serverPort = 8000;
        syncword = 0x2dd4;
    }
    else
    {
        identifier = (unsigned int)strtol(argv[1], NULL, 16);
        serverIp = argv[2];
        serverPort = atoi(argv[3]);
        syncword = (short)strtol(argv[4], NULL, 16);
    }


    printf("identifier is 0x%.2x\n", identifier);
    printf("server ip is %s\n", serverIp.c_str());
    printf("server port is %d\n", serverPort);
    printf("syncword is0x%.2x\n", syncword);

    map<short, Unit *> mainUnitMap;

    sem_t mapLock;
    sem_init(&mapLock, 0, 1);

    Record record;
    record.setFile("/App/metersData");



    Net netCenter;
    netCenter.setIdentifier(identifier);
    netCenter.setIpPort(serverIp, serverPort);
    netCenter.setUnitMap(&mainUnitMap, &mapLock);
    netCenter.setRecordPointer(&record);
    netCenter.start();

    Spi spi;
    spi.setDevices("/dev/spidev1.0", "/dev/my_led");
    spi.chipOpen();

    Si4432 si4432;
    si4432.setSpi(&spi);
    si4432.reset();
    si4432.init(syncword);
    si4432.setIdleMode();

    Reader mainReader;
    mainReader.setUnitMap(&mainUnitMap, &mapLock);
    mainReader.setSi4432(&si4432);
    mainReader.start();

    while(true);

    record.closeFile();

    return 0;
}

