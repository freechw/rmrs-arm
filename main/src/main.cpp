#include "unit.h"
#include "net.h"
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

    map<short, Unit *> mainUnitMap;



    Net netCenter;
    netCenter.setIdentifier(0x11223344);
    netCenter.setIpPort("192.168.1.158", 8000);
    netCenter.setUnitMap(&mainUnitMap);
    netCenter.start();

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

    while(true);

    return 0;
}
