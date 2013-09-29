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

    map<short, Unit *> mainUnitMap;

    sem_t mapLock;
    sem_init(&mapLock, 0, 1);



    Net netCenter;
    netCenter.setIdentifier(0x11223344);
    netCenter.setIpPort("192.168.1.158", 8000);
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

