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
    if (argc != 4)
    {
        printf("error args!\n");
        identifier = 0x11223344;
        serverIp = "192.168.1.158";
        serverPort = 8000;
    }
    else
    {
        char * buf = argv[1];
        for (int i = 0; i < 8; i++)
        {
            identifier += (((unsigned int)buf[i] -0x30) * (unsigned int)pow(16, (7-i)));
        }

        serverIp = argv[2];
        serverPort = atoi(argv[3]);
    }


    printf("identifier is 0x%.2x\n", identifier);
    printf("server ip is %s\n", serverIp.c_str());
    printf("server port is %d\n", serverPort);

    map<short, Unit *> mainUnitMap;

    sem_t mapLock;
    sem_init(&mapLock, 0, 1);



    Net netCenter;
    netCenter.setIdentifier(identifier);
    netCenter.setIpPort(serverIp, serverPort);
    netCenter.setUnitMap(&mainUnitMap, &mapLock);
    netCenter.start();

    Spi spi;
    spi.setDevices("/dev/spidev1.0", "/dev/my_led");
    spi.chipOpen();

    Si4432 si4432;
    si4432.setSpi(&spi);
    si4432.reset();
    si4432.init();
    si4432.setIdleMode();

    Reader mainReader;
    mainReader.setUnitMap(&mainUnitMap, &mapLock);
    mainReader.setSi4432(&si4432);
    mainReader.start();


    while(true);

    return 0;
}

