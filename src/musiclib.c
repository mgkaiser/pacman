#include <6502.h>
#include <stdlib.h>
#include <cc65.h>
#include <c64.h>
#include "musiclib.h"

char *musicData = (char *)0xc000;

void copyMusic(unsigned int musicSourceLen, char* musicSource)
{
    static unsigned int x;    

    for (x = 0; x <= musicSourceLen; ++x)
    {
        musicData[x] = musicSource[x];        
    }    
}