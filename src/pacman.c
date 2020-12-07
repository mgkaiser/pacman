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
#include "actors.h"
#include "pacman.h"

#define STACK_SIZE 16
unsigned char stackSize[STACK_SIZE];

#define SPRITE_BASE 0x80
char *screenData    = (char *)0xb800;   // 0xe0: "(Address - VICBase ) / 0x40" -> "(0xb800 - 0x8000) / 0x40"
char *tileData      = (char *)0xb000;   // 0x0c: "(Address - VICBase ) / 0x400" -> "(0xb800 - 0x8000) / 0x400" ** ONLY EVEN ARE VALID **
char *colorData     = (char *)0xd800;
char *spriteData    = (char *)0xa000;
char *spriteSlot    = (char *)0xbbf8;

unsigned char interuptCounter;
unsigned char ghostDoorOpen;
unsigned char interrupt(void)
{                   
    interuptCounter++;
    if (ghostDoorOpen > 0) ghostDoorOpen--;

    renderActor(&actor_Player);
    renderActor(&actor_Ghost1);
    renderActor(&actor_Ghost2);
    renderActor(&actor_Ghost3);
    renderActor(&actor_Ghost4);

    //checkActorPlayer(&actor_Player);
    //checkActorGhost(&actor_Ghost1, 1, 1);
    //checkActorGhost(&actor_Ghost2, 1, 0);
    //checkActorGhost(&actor_Ghost3, 0, 1);
    //checkActorGhost(&actor_Ghost4, 0, 0);       

    moveActorPlayer(&actor_Player);
    moveActorGhost(&actor_Ghost1, 1, 1);
    moveActorGhost(&actor_Ghost2, 1, 0);
    moveActorGhost(&actor_Ghost3, 0, 1);
    moveActorGhost(&actor_Ghost4, 0, 0);       
    
    // Acknowlege the interrrupt 
    VIC.irr++;

    return IRQ_HANDLED;                         
}

void initInterrupt (void)
{
    unsigned short dummy;     

    interuptCounter = 0;
    ghostDoorOpen = 0xff;
    
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

void copyScreen(void)
{
    unsigned int x;

    for (x = 0; x < maze_char_bin_len; x ++)
    {
        screenData[x] = maze_char_bin[x];
        colorData[x] = maze_colors_bin[x];                
    }
}

void copyChars(void)
{
    unsigned int x;

    for (x = 0; x < maze_tiles_chr_len; x ++)
    {
        tileData[x] = maze_tiles_chr[x];        
    }
}

void copySprites()
{
    unsigned int x;

    for (x = 0; x < pacman_sprites_spr_len; x ++)
    {
        spriteData[x] = pacman_sprites_spr[x];        
    }
}

void initSprites(void)
{    
    actor_Ghost1.spriteNumber = 0;
    actor_Ghost1.frames = GHOST_FRAMES;
    actor_Ghost1.frame = 0;    
    actor_Ghost1.animationDelay = 0;
    actor_Ghost1.animationDelayMax = 8;

    actor_Ghost2.spriteNumber = 1;
    actor_Ghost2.frames = GHOST_FRAMES;
    actor_Ghost2.frame = 0;    
    actor_Ghost2.animationDelay = 0;
    actor_Ghost2.animationDelayMax = 8;

    actor_Ghost3.spriteNumber = 2;
    actor_Ghost3.frames = GHOST_FRAMES;
    actor_Ghost3.frame = 0;    
    actor_Ghost3.animationDelay = 0;
    actor_Ghost3.animationDelayMax = 8;

    actor_Ghost4.spriteNumber = 3;
    actor_Ghost4.frames = GHOST_FRAMES;
    actor_Ghost4.frame = 0;    
    actor_Ghost4.animationDelay = 0;
    actor_Ghost4.animationDelayMax = 8;

    actor_Player.spriteNumber = 4;
    actor_Player.frames = PLAYER_FRAMES;
    actor_Player.frame = 0;    
    actor_Player.animationDelay = 0;
    actor_Player.animationDelayMax = 6;
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
    // Enable sprites
    VIC.spr_ena = 0xff;
  
    //Sprites 0-3 multicolor, 4-7 high resolution
    VIC.spr_mcolor = 0x0f;

    // Set the 2nd and 3rd sprite colors white and black
    VIC.spr_mcolor0 = COLOR_WHITE;
    VIC.spr_mcolor1 = COLOR_BLACK;  
  
    // Ghost 1 (Red) Initial Postion, above the box    
    VIC.spr0_color = COLOR_RED;
    actor_Ghost1.x = 0x81;
    actor_Ghost1.y = 0x57;   
    actor_Ghost1.dx = 1;
    actor_Ghost1.dy = 0;     
    actor_Ghost1.framedata = (char*)&animation_ghost_left_up;      

    // Ghost 2 (Pink) Initial Postion, in the box
    VIC.spr1_color = COLOR_LIGHTRED;
    actor_Ghost2.x = 0x89;
    actor_Ghost2.y = 0x77;   
    actor_Ghost2.dx = 0;
    actor_Ghost2.dy = -1;     
    actor_Ghost2.framedata = (char*)&animation_ghost_right_up;    

    // Ghost 3 (Orange) Initial Postion, in the box
    VIC.spr2_color = COLOR_ORANGE;
    actor_Ghost3.x = 0x79;
    actor_Ghost3.y = 0x77;   
    actor_Ghost3.dx = 0;
    actor_Ghost3.dy = 1;     
    actor_Ghost3.framedata = (char*)&animation_ghost_left_down;    

    // Ghost 4 (Cyan) Initial Postion, in the box
    VIC.spr3_color = COLOR_CYAN;
    actor_Ghost4.x = 0x81;
    actor_Ghost4.y = 0x87;   
    actor_Ghost4.dx = 0;
    actor_Ghost4.dy = -1;     
    actor_Ghost4.framedata = (char*)&animation_ghost_right_down;              
}

void initPlayer(void)
{
    // Player
    VIC.spr4_color = COLOR_YELLOW;
    actor_Player.x = 0x85;
    actor_Player.y = 0xB7;   
    actor_Player.dx = 1;
    actor_Player.dy = 0;     
    actor_Player.framedata = (char*)&animation_player_right;
}

int main (void)
{
    // Copy the graphics to where they belong
    copyScreen();
    copyChars();
    copySprites();

    // Initialize the ghosts and players
    initSprites();
    initGhosts();    
    initPlayer();

    // Setup the video card
    initVic();    

    // Kick off the interrupt
    initInterrupt();    
    
    // Wait forever - Read joystick, move player.
    while(1);

    return EXIT_SUCCESS;        
}
