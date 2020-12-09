/*****************************************************************************\
** Pacman                                                                    **
** (c)2020 by mkaiser                                                        **
\*****************************************************************************/

#include <6502.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <stdio.h>
#include <cc65.h>
#include <c64.h>
#include "maze_tiles.h"
#include "maze_char.h"
#include "maze_colors.h"
#include "pacman_sprites.h"
#include "actors.h"
#include "pacman.h"
#include "intro_music.h"

// Size of stack for the interrupt
#define STACK_SIZE 16
// Stack for the interrupt
unsigned char stackSize[STACK_SIZE];

// First sprite page in our sprite data
#define SPRITE_BASE 0x80

// Pointer to the screen buffer
char *screenData    = (char *)0xb800;   
// Pointer to the custom characters
char *tileData      = (char *)0xb000; 
// Pointer to the color buffer  
char *colorData     = (char *)0xd800;
// Pointer to the sprite data
char *spriteData    = (char *)0xa000;
// Pointer to the sprite slots
char *spriteSlot    = (char *)0xbbf8;
//
char *musicData     =(char *)0x9c5f;

// Score
unsigned long score1;
unsigned long score2;
unsigned int dotsRemaining;
unsigned int pillsRemaining;
char *stringTemp = "                 ";

// Counter that increments every frame
unsigned char interuptCounter;
unsigned char frameTrigger;
unsigned int waitCounter;

unsigned int scan_start;
unsigned int scan_afterdraw;
unsigned int scan_aftermusic;
unsigned int scan_done;

// Interrupt handler called once per frame
unsigned char interrupt(void)
{   
    scan_start = VIC.rasterline;

    ++interuptCounter;   
    
    // Draw the ghosts
    renderActor(&actor_Player);
    renderActor(&actor_Ghost1);
    renderActor(&actor_Ghost2);
    renderActor(&actor_Ghost3);
    renderActor(&actor_Ghost4);   

    scan_afterdraw = VIC.rasterline;

    // Play tune                                   
    //__asm__ ("jsr $9c7a");      
    if ((interuptCounter & 0x01) == 1) {
        __asm__ ("jsr $9c7a");                         
    }
    
    scan_aftermusic = VIC.rasterline;
    
    // Wait
    if (waitCounter != 0)
    {               
        --waitCounter;
    }   

    // Normal loop
    else
    {   
        // If all the dot are gone, stop and wait!       
        if (pillsRemaining == 0 && dotsRemaining == 0) 
        {
            waitCounter = 0xffff;
            actor_Player.framedata = (char*)&animation_player_still;
        }
        
        // See if the player did anything good
        checkActorPlayer(&actor_Player);
        
        // Check for their next movement
        moveActorPlayer(&actor_Player);
        moveActorGhost(&actor_Ghost1);
        moveActorGhost(&actor_Ghost2);
        moveActorGhost(&actor_Ghost3);
        moveActorGhost(&actor_Ghost4);                           
    }

    // Acknowlege the interrrupt 
    VIC.irr = 1;    

    // Tell main thread we drew a frame
    frameTrigger=1;

    scan_done = VIC.rasterline;

    return IRQ_HANDLED;                         
}

// Setup the interrupt handler
void initInterrupt (void)
{
    unsigned short dummy;     

    interuptCounter = 0;    
    frameTrigger = 0;
    waitCounter = 0xffff; 
    
    SEI();    
    CIA1.icr = 0x7F;                                // Turn of CIA timer
    VIC.ctrl1 = (VIC.ctrl1 & 0x7F);                 // Clear MSB of raster
    dummy = CIA1.icr;                               // Acknowlege any outstaiding interrupts from CIA1
    dummy = CIA2.icr;                               // Acknowlege any outstaiding interrupts from CIA1
    VIC.rasterline = 230;                           // Set raster line
    set_irq(&interrupt, stackSize, STACK_SIZE);     // Set the interrupt handler
    VIC.imr = 0x01;                                 // Enable the VIC raster interrupt    
    CLI();    
}

// Copy the music data to the buffers
void copyMusic(void)
{
    static unsigned int x;
    static unsigned int y;

    for (x = 0, y = Pac_Man_sid_start; y < Pac_Man_sid_len ; ++x, ++y)
    {
        musicData[x] = Pac_Man_sid[y];        
    }
}

// Copy the screen data to the buffers
void copyScreen(void)
{
    unsigned int x;

    for (x = 0; x < maze_char_bin_len; x ++)
    {
        screenData[x] = maze_char_bin[x];
        colorData[x] = maze_colors_bin[x];                
    }
}

// Copy the custom characters to the buffers
void copyChars(void)
{
    unsigned int x;

    for (x = 0; x < maze_tiles_chr_len; x ++)
    {
        tileData[x] = maze_tiles_chr[x];        
    }
}

// Copy the sprite data to the buffers
void copySprites()
{
    unsigned int x;

    for (x = 0; x < pacman_sprites_spr_len; x ++)
    {
        spriteData[x] = pacman_sprites_spr[x];        
    }
}

// Setup the video chip
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

// Set ghosts to initial state
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
    actor_Ghost1.spriteNumber = 0;
    actor_Ghost1.frames = GHOST_FRAMES;
    actor_Ghost1.frame = 0;    
    actor_Ghost1.animationDelay = 0;
    actor_Ghost1.animationDelayMax = 8;
    actor_Ghost1.moveDelay = 255;
    actor_Ghost1.moveDelayMax = 255;
    actor_Ghost1.ghostDoorOpen = 0;
    actor_Ghost1.x = 0x81;
    actor_Ghost1.y = 0x57;   
    actor_Ghost1.dx = 1;
    actor_Ghost1.dy = 0;     
    actor_Ghost1.aggressivex = 1;
    actor_Ghost1.aggressivey = 1;
    actor_Ghost1.framedata = (char*)&animation_ghost_left_up;      
    

    // Ghost 2 (Pink) Initial Postion, in the box
    VIC.spr1_color = COLOR_LIGHTRED;
    actor_Ghost2.spriteNumber = 1;
    actor_Ghost2.frames = GHOST_FRAMES;
    actor_Ghost2.frame = 0;    
    actor_Ghost2.animationDelay = 0;
    actor_Ghost2.animationDelayMax = 8;
    actor_Ghost2.moveDelay = 30;
    actor_Ghost2.moveDelayMax = 30;
    actor_Ghost2.ghostDoorOpen = 100;
    actor_Ghost2.x = 0x89;
    actor_Ghost2.y = 0x87;   
    actor_Ghost2.dx = 0;
    actor_Ghost2.dy = -1;     
    actor_Ghost2.aggressivex = 1;
    actor_Ghost2.aggressivey = 0;
    actor_Ghost2.framedata = (char*)&animation_ghost_right_up;    

    // Ghost 3 (Orange) Initial Postion, in the box
    VIC.spr2_color = COLOR_ORANGE;
    actor_Ghost3.spriteNumber = 2;
    actor_Ghost3.frames = GHOST_FRAMES;
    actor_Ghost3.frame = 0;    
    actor_Ghost3.animationDelay = 0;
    actor_Ghost3.animationDelayMax = 8;
    actor_Ghost3.moveDelay = 30;
    actor_Ghost3.moveDelayMax = 30;
    actor_Ghost3.ghostDoorOpen = 180;
    actor_Ghost3.x = 0x79;
    actor_Ghost3.y = 0x87;   
    actor_Ghost3.dx = 0;
    actor_Ghost3.dy = -1;     
    actor_Ghost3.aggressivex = 0;
    actor_Ghost3.aggressivey = 1;
    actor_Ghost3.framedata = (char*)&animation_ghost_left_down;    

    // Ghost 4 (Cyan) Initial Postion, in the box
    VIC.spr3_color = COLOR_CYAN;
    actor_Ghost4.spriteNumber = 3;
    actor_Ghost4.frames = GHOST_FRAMES;
    actor_Ghost4.frame = 0;    
    actor_Ghost4.animationDelay = 0;
    actor_Ghost4.animationDelayMax = 8;
    actor_Ghost4.moveDelay = 15;
    actor_Ghost4.moveDelayMax = 15;
    actor_Ghost4.ghostDoorOpen = 255; 
    actor_Ghost4.x = 0x81;
    actor_Ghost4.y = 0x87;   
    actor_Ghost4.dx = 0;
    actor_Ghost4.dy = -1;     
    actor_Ghost4.aggressivex = 0;
    actor_Ghost4.aggressivey = 0;
    actor_Ghost4.framedata = (char*)&animation_ghost_right_down;              
}

// Set player to initial state
void initPlayer(void)
{
    // Player
    VIC.spr4_color = COLOR_YELLOW;
    actor_Player.spriteNumber = 4;
    actor_Player.frames = PLAYER_FRAMES;
    actor_Player.frame = 0;    
    actor_Player.animationDelay = 0;
    actor_Player.animationDelayMax = 6;
    actor_Player.moveDelay = 30;
    actor_Player.moveDelayMax = 30;   
    actor_Player.x = 0x85;
    actor_Player.y = 0xB7;   
    actor_Player.dx = 1;
    actor_Player.dy = 0;     
    actor_Player.framedata = (char*)&animation_player_still;
}

void showScore(void)
{
    sprintf(stringTemp, "1up %06lu", score1);
    draw_string(29, 1, 12, stringTemp);

    sprintf(stringTemp, "2up %06lu", score2);
    draw_string(29, 3, 12, stringTemp);    
}

void showDebug(void)
{
    sprintf(stringTemp, "st %06d", scan_start);
    draw_string(29, 5, 12, stringTemp);    
    sprintf(stringTemp, "ad %06d", scan_afterdraw);
    draw_string(29, 6, 12, stringTemp);    
    sprintf(stringTemp, "am %06d", scan_aftermusic);
    draw_string(29, 7, 12, stringTemp);    
    sprintf(stringTemp, "do %06d", scan_done);
    draw_string(29, 8, 12, stringTemp);        

    /*
    sprintf(stringTemp, "g1 %06d", actor_Ghost1.moveDelay);
    draw_string(29, 10, 12, stringTemp);        
    sprintf(stringTemp, "g2 %06d", actor_Ghost2.moveDelay);
    draw_string(29, 11, 12, stringTemp);        
    sprintf(stringTemp, "g3 %06d", actor_Ghost3.moveDelay);
    draw_string(29, 12, 12, stringTemp);        
    sprintf(stringTemp, "g4 %06d", actor_Ghost4.moveDelay);
    draw_string(29, 13, 12, stringTemp);        
    sprintf(stringTemp, "p1 %06d", actor_Player.moveDelay);
    draw_string(29, 14, 12, stringTemp);        
    */
    
}

void showReady()
{
    // Double size
    VIC.spr_exp_x = 0xe0;
    VIC.spr_exp_y = 0xe0;

    // Set images
    spriteSlot[5] = 0x97;
    spriteSlot[6] = 0x98;

    // Move sprit1es to
    VIC.spr5_x = 95;
    VIC.spr5_y = 110;
    VIC.spr6_x = 143;
    VIC.spr6_y = 110;    

    // Set Color
    VIC.spr5_color = COLOR_YELLOW;
    VIC.spr6_color = COLOR_YELLOW;
    
    // Enable sprites
    VIC.spr_ena = 0x7f;

}

void hideReady()
{
    // Disable sprites
    VIC.spr_ena = 0x1f;
}

void showGameOver()
{
    // Double size
    VIC.spr_exp_x = 0xe0;
    VIC.spr_exp_y = 0xe0;

    // Set images
    spriteSlot[5] = 0x99;
    spriteSlot[6] = 0x9a;
    spriteSlot[7] = 0x9b;

    // Move sprit1es to    
    VIC.spr5_x = 77;
    VIC.spr5_y = 110;
    VIC.spr6_x = 125;
    VIC.spr6_y = 110;
    VIC.spr7_x = 173;
    VIC.spr7_y = 110;        

    // Set Color
    VIC.spr5_color = COLOR_YELLOW;
    VIC.spr6_color = COLOR_YELLOW;
    VIC.spr7_color = COLOR_YELLOW;
    
    // Enable sprites
    VIC.spr_ena = 0xff;     
}

void hideGameOver()
{
    // Disable sprites
    VIC.spr_ena = 0x1f;
}

void resetLevel(unsigned char playTune)
{
            
    // Maybe play tune
    if (playTune == 1)
    {
        // Init Music
        SEI();
        __asm__ ("lda #00");
        __asm__ ("tax");
        __asm__ ("tay");
        __asm__ ("jsr $9c5f");        
        CLI();        
        waitCounter = 570; 
    }
    else
    {
        waitCounter = 180; 
    }            

    // Display Ready Message  
    showReady();      

    // Reset the game state
    copyScreen();
    initGhosts();    
    initPlayer();
    pillsRemaining = 4;
    dotsRemaining = 218;
    // Unscare Ghosts
    showScore();
    
    // Wait a bit        
    while(waitCounter != 0) {}        

    // Clear the ready message    
    hideReady();
}

// Main entry point
int main (void)
{        
    // Copy the graphics to where they belong
    copyChars();
    copySprites();
    copyMusic();

    // Setup the video card
    initVic();    

    // Kick off the interrupt    
    initInterrupt();    

    // Initialize the ghosts and players    
    resetLevel(1);    
        
    // Wait forever - Read joystick, move player.
    while(1)
    {
        // Wait for the frame to trigger 
        while(frameTrigger == 0);

        // Show the score 
        showScore();
        showDebug();

        // Reset if the player ate everything
        if (pillsRemaining == 0 && dotsRemaining == 0) 
        {
            waitCounter = 180;
            while(waitCounter != 0);
            resetLevel(0);        
        }
        
        // Acknowledge the frame
        frameTrigger = 0;
    }

    return EXIT_SUCCESS;        
}


