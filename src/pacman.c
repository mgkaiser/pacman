/*****************************************************************************\
** Pacman                                                                    **
** (c)2020 by mkaiser                                                        **
\*****************************************************************************/

#include <6502.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <cc65.h>
#include <c64.h>
#include "maze_tiles.h"
#include "maze_char.h"
#include "maze_colors.h"
#include "pacman_sprites.h"

#define STACK_SIZE 16
unsigned char stackSize[STACK_SIZE];

#define SPRITE_BASE = 0x2000 / 0x40
char *screenData    = (char *)0xb800;   // 0xe0: "(Address - VICBase ) / 0x40" -> "(0xb800 - 0x8000) / 0x40"
char *tileData      = (char *)0xb000;   // 0x0c: "(Address - VICBase ) / 0x400" -> "(0xb800 - 0x8000) / 0x400" ** ONLY EVEN ARE VALID **
char *colorData     = (char *)0xd800;
char *spriteData    = (char *)0xa000;

unsigned char interrupt(void)
{       
    

    return IRQ_NOT_HANDLED;                         
}

void initInterrupt (void)
{
    unsigned short dummy;     
    
    SEI();    
    CIA1.icr = 0x7F;                                // Turn of CIA timer
    VIC.ctrl1 = (VIC.ctrl1 & 0x7F);                 // Clear MSB of raster
    dummy = CIA1.icr;                               // Acknowlege any outstaiding interrupts from CIA1
    dummy = CIA2.icr;                               // Acknowlege any outstaiding interrupts from CIA1
    VIC.rasterline = 220;                           // Set raster line
    set_irq(&interrupt, stackSize, STACK_SIZE);     // Set the interrupt handler
    VIC.imr = 0x01;                                 // Enable the VIC raster interrupt
    CLI();
}

void initScreen(void)
{
    unsigned int x;

    for (x = 0; x < maze_char_bin_len; x ++)
    {
        screenData[x] = maze_char_bin[x];
        colorData[x] = maze_colors_bin[x];                
    }
}

void initChars(void)
{
    unsigned int x;

    for (x = 0; x < maze_tiles_chr_len; x ++)
    {
        tileData[x] = maze_tiles_chr[x];        
    }
}

void initSprites(void)
{
    unsigned int x;

    for (x = 0; x < pacman_sprites_spr_len; x ++)
    {
        spriteData[x] = pacman_sprites_spr[x];        
    }
}

void copySprites()
{
unsigned int x;

    for (x = 0; x < 0x0800; x ++)
    {
        tileData[x] = maze_tiles_chr[x];        
    }
}

void initVic(void)
{
    // Pointer to 0/1, the CPU control register
    char *cpu = (char *)0x0000;

    // Page out the Basic ROM
    cpu[1] = (cpu[1] & 0xfe);
  
    // Black background
    VIC.bgcolor0 = COLOR_BLACK;
    VIC.bordercolor = COLOR_BLACK;
  
    // Change VIC banks
    CIA2.pra = 0x01;  

    // Relocate the character set
    VIC.addr = 0xec;
}

void initGhosts(void)
{
    
}

int main (void)
{
    // Kick everything off
    initScreen();
    initChars();
    copySprites();
    initGhosts();
    initSprites();
    initVic();    
    initInterrupt();    
    
    // Wait forever
    while(1);

    return EXIT_SUCCESS;        
}
