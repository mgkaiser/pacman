#include <6502.h>
#include <stdlib.h>
#include <stdio.h>
#include <cc65.h>
#include <c64.h>

// First sprite page in our sprite data
#define SPRITE_BASE 0x80

char *screenData    = (char *)0xb800;   
char *tileData      = (char *)0xb000; 
char *colorData     = (char *)0xd800;
char *spriteData    = (char *)0xa000;
char *spriteSlot    = (char *)0xbbf8;

char buf[20];

// Convert sprite x coord to screen x coord
unsigned char spritexToscreenx(unsigned char spritex)
{
    return (spritex - 21) / 8;
}

// Convert screen x coord to sprite x coord
unsigned char screenxTospritex(unsigned char screenx)
{
    return (screenx * 8) + 21;
}

// Convert sprite y coord to screen y coord
unsigned char spriteyToscreeny(unsigned char spritey)
{
    return (spritey - 47) / 8;
}

// Convert screen y coord to sprite y coord
unsigned char screenyTospritey(unsigned char screeny)
{
    return (screeny * 8) + 47;
}

// Return the offset in the screen "array" for the character at x, y
unsigned int  screenxyToAddress(unsigned char screenx, unsigned char screeny)
{
    return (screeny * 40) + screenx;
}

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