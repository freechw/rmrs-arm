#include "unit.h"
#include <stdio.h>
#include <vector>

using std::vector;

int main()
{
    Unit unit1;
    unit1.setUnitId(0x0101);
    for (int i = 1; i < 8; i++)
    {
        unit1.addMeterId(i);
    }

    unit1.setCurrentLineCleared();

    vector<int> tmpLine;
    tmpLine = unit1.getCurrentMeterIds();
//    int f = tmpLine.front();
//    printf(" %d\n", f);
    vector<int>::const_iterator iter = tmpLine.begin();
    while (tmpLine.end() != iter)
    {
        printf(" %d", *iter);
        iter++;
    }
    printf("\n");

    unit1.startSender();

    for (int i = 0; i < 4; i++)
    {
        unsigned char * pchar = NULL;
        unit1.addReadyMeterData(pchar);
    }

    while(true);

    return 0;
}

