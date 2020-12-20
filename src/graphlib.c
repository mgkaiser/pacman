#include <6502.h>
#include <stdlib.h>
#include <stdio.h>
#include <cc65.h>
#include <c64.h>
#include "graphlib.h"

// First sprite page in our sprite data
#define SPRITE_BASE 0x80

char *screenData    = (char *)0xb800;   
char *tileData      = (char *)0xb000; 
char *colorData     = (char *)0xd800;
char *spriteData    = (char *)0xa000;
char *spriteSlot    = (char *)0xbbf8;

char buf[20];

unsigned int times40[] = {0x0,0x28,0x50,0x78,0xA0,0xC8,0xF0,0x118,0x140,0x168,0x190,0x1B8,0x1E0,0x208,0x230,0x258,0x280,0x2A8,0x2D0,0x2F8,0x320,0x348,0x370,0x398,0x3C0};
unsigned int times8[] = {0x0,0x8,0x10,0x18,0x20,0x28,0x30,0x38,0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,0x80,0x88,0x90,0x98,0xA0,0xA8,0xB0,0xB8,0xC0,0xC8,0xD0,0xD8,0xE0,0xE8,0xF0,0xF8,0x100,0x108,0x110,0x118,0x120,0x128,0x130,0x138,0x140};

// Copy the screen data to the buffers
void copyScreen(unsigned int screenDataLen, char* scrnData, char* colData)
{
    static unsigned int x;

    for (x = 0; x < screenDataLen; x ++)
    {
        screenData[x] = scrnData[x];
        colorData[x] = colData[x];                
    }
}

// Copy the custom characters to the buffers
void copyChars(unsigned int charDataLen, char* charData) 
{
    static unsigned int x;

    for (x = 0; x < charDataLen; x ++)
    {
        tileData[x] = charData[x];        
    }
}

// Copy the sprite data to the buffers 
void copySprites(unsigned int sprDataLen, char* sprData) 
{
    static unsigned int x;
    for (x = 0; x < sprDataLen; x ++)
    {
        spriteData[x] = sprData[x];        
    }
}

void  draw_string(unsigned char x, unsigned char y, unsigned char w, char *ch)
{
	static unsigned char xctr;		
			
	char ch2;
		
	for(xctr = 0; xctr < w; ++xctr)
	{	
		ch2 = ch[xctr];
        if (ch2 == 0x00) break;
		
		if (ch[xctr] <= 0x1f) ch2 = ch2 + 0x80;
		if (ch[xctr] >= 0x20 & ch[xctr] <= 0x3f) ch2 = ch2 + 0x00;
		if (ch[xctr] >= 0x40 & ch[xctr] <= 0x5f) ch2 = ch2 + 0xc0;
		if (ch[xctr] >= 0x60 & ch[xctr] <= 0x7f) ch2 = ch2 + 0xe0;
		if (ch[xctr] >= 0x80 & ch[xctr] <= 0x9f) ch2 = ch2 + 0x40;
		if (ch[xctr] >= 0xa0 & ch[xctr] <= 0xbf) ch2 = ch2 + 0xc0;
		if (ch[xctr] >= 0xc0 & ch[xctr] <= 0xdf) ch2 = ch2 + 0x80;
		if (ch[xctr] >= 0xe0 & ch[xctr] <= 0xfe) ch2 = ch2 + 0x80;
		if (ch[xctr] == 0xff) ch2 = 0x5e;
		
        screenData[screenxyToAddress(x + xctr,y)] = ch2;		
	}
	
}

void  draw_string_char(unsigned char x, unsigned char y, unsigned char ch)
{
    sprintf(buf, "%d", ch);
    draw_string(x, y , 0x04, buf);
}