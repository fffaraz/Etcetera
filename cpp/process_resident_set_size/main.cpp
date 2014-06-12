#include <iostream>
#include "getRSS.h"

using namespace std;

int main()
{
    size_t currentSize0 = getCurrentRSS( );
    size_t peakSize0    = getPeakRSS( );

    int** a = new int*[10240];

    for(int i=0; i<10240; i++)
        a[i] = new int[10240];

    size_t currentSize1 = getCurrentRSS( );
    size_t peakSize1    = getPeakRSS( );

    for(int i=0; i<10240; i++)
        for(int j=0; j<10240; j++)
            a[i][j] = 0x12345678;

    size_t currentSize2 = getCurrentRSS( );
    size_t peakSize2    = getPeakRSS( );

    for(int i=0; i<10240; i++)
        delete[] a[i];

    delete[] a;

    size_t currentSize3 = getCurrentRSS( );
    size_t peakSize3    = getPeakRSS( );

    cout << "currentSize 0 : " << currentSize0 << endl;
    cout << "peakSize 0    : " << peakSize0    << endl;
    cout << "currentSize 1 : " << currentSize1 << endl;
    cout << "peakSize 1    : " << peakSize1    << endl;
    cout << "currentSize 2 : " << currentSize2 << endl;
    cout << "peakSize 2    : " << peakSize2    << endl;
    cout << "currentSize 3 : " << currentSize3 << endl;
    cout << "peakSize 3    : " << peakSize3    << endl;

    cout << endl;
    cout << "currentSize : " << currentSize3 - currentSize0 << endl;
    cout << "peakSize    : " << peakSize3 - peakSize0       << endl;

    return 0;
}

