#include <6502.h>
#include <c64.h>
#include <stdlib.h>
#include "actors.h"
#include "pacman.h"
#include "graphlib.h"
#include "musiclib.h"
#include "sfx.h"

// Describes the ghost animations
unsigned char animation_ghost_left_up[]     = { 0x80, 0x84 };
unsigned char animation_ghost_right_up[]    = { 0x81, 0x85 };
unsigned char animation_ghost_left_down[]   = { 0x82, 0x86 };
unsigned char animation_ghost_right_down[]  = { 0x83, 0x87 };
unsigned char animation_ghost_scared[]      = { 0x91, 0x91 };
unsigned char animation_ghost_dead[]        = { 0x96, 0x96 };
unsigned char animation_ghost_1600[]        = { 0x95, 0x95 };
unsigned char animation_ghost_800[]         = { 0x94, 0x94 };
unsigned char animation_ghost_400[]         = { 0x93, 0x93 };
unsigned char animation_ghost_200[]         = { 0x92, 0x92 };

// Describes the player animations
unsigned char animation_player_right[]      = { 0x88, 0x89, 0x8a, 0x89 };
unsigned char animation_player_down[]       = { 0x88, 0x8d, 0x8e, 0x8d };
unsigned char animation_player_up[]         = { 0x88, 0x8b, 0x8c, 0x8b };
unsigned char animation_player_left[]       = { 0x88, 0x8f, 0x90, 0x8f };
unsigned char animation_player_still[]      = { 0x88, 0x88, 0x88, 0x88 };
unsigned char animation_player_die[]        = { 0x8b, 0x8b, 0x8c, 0x8c, 0x9c, 0x9c, 0x9e, 0x9e, 0x9f, 0x9f, 0xa0, 0xa0, 0xa1, 0xa1, 0xa2, 0xa2, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3};

// The actors
struct actor actor_Ghost1;
struct actor actor_Ghost2;
struct actor actor_Ghost3;
struct actor actor_Ghost4;
struct actor actor_Player;

// Are we blocked at the specified coords
unsigned char fastcall isBlocked(unsigned char screenx, unsigned char screeny)
{
    static unsigned char charToCheck;
    charToCheck = screenData[screenxyToAddress(screenx, screeny)];            
    if (charToCheck == 0x2E) return 0;
    if (charToCheck == 0x51) return 0;
    if (charToCheck == 0x20) return 0;     
    return 1;
}

unsigned char fastcall isBlockedGhostDoorOpen(unsigned char screenx, unsigned char screeny)
{
    static unsigned char charToCheck;
    charToCheck = screenData[screenxyToAddress(screenx, screeny)];        
    if (charToCheck == 0x2E) return 0;
    if (charToCheck == 0x51) return 0;
    if (charToCheck == 0x20) return 0;
    if (charToCheck == 0x79) return 0;    
    return 1;
}

// Make the ghosts look at the player
unsigned char* fastcall LookTowardPlayer(unsigned char x, unsigned char y)
{
    if (actor_Player.x < x)
    {
        if (actor_Player.y < y)
        {
            // up/left
            return (char *)&animation_ghost_left_up;  
        }
        else
        {
            // down/left
            return (char *)&animation_ghost_left_down;  
        }        
    }
    else
    {
        if (actor_Player.y < y)
        {
            // up/right
            return (char *)&animation_ghost_right_up;  
        }
        else
        {
            // down/right
            return (char *)&animation_ghost_right_down;  
        }
    }
    
}

// Take the data from the struct and actually draw the actor
void fastcall renderActor(register struct actor *pActor)
{   
    static unsigned char spriteNumber;
    
    spriteNumber = pActor->spriteNumber;

    VIC.spr_pos[spriteNumber].x = pActor->x;
    VIC.spr_pos[spriteNumber].y = pActor->y;

    if (pActor->multicolor)
    {
        VIC.spr_mcolor = (VIC.spr_mcolor) | (pActor->positiveMask);        
    }
    else
    {
        VIC.spr_mcolor = (VIC.spr_mcolor) & (pActor->negativeMask);
    }

    if (pActor->animationDelay-- == 0)
    {
        spriteSlot[spriteNumber] = *(pActor->framedata + pActor->frame);              
        ++(pActor->frame);
        if (pActor->frame >= pActor->frames) pActor->frame = 0;
        pActor->animationDelay = pActor->animationDelayMax;
    }
}

// Move the actor in the selected direction unless blocked
void fastcall moveActorGhost(register struct actor *pActor)
{
    static unsigned char screenx;
    static unsigned char screeny;  
    static unsigned char aggressivex;
    static unsigned char aggressivey;
    static unsigned char isItBlocked;
    static unsigned char targetx;
    static unsigned char targety;

    if (playerDied) return;
               
    if (pActor->moveDelayMax == 255 || --(pActor -> moveDelay) != 1)
    {
        if (pActor->ghostDoorOpen != 0) {
            pActor->ghostDoorOpen = pActor->ghostDoorOpen - 1;
            pActor->suppressAggression = 150;
        }
        
        if (pActor->suppressAggression != 0) --(pActor->suppressAggression); 
    
        if (pActor->ghostDead)
        {
            aggressivex = 1;
            aggressivey = 1;
        }
        else if (pActor->suppressAggression != 0 || pActor->ghostScared > 0)
        {
            aggressivex = 0;
            aggressivey = 0;
        }
        else
        {
            aggressivex = pActor->aggressivex;
            aggressivey = pActor->aggressivey;
        }

        if (pActor->ghostDead)
        {
            targetx = 0x81;
            targety = 0x57;
            if (pActor->x == targetx && pActor->y == targety)
            {
                pActor->ghostDoorOpen = 240;
                pActor->x = 0x89;
                pActor->y = 0x87;   
                pActor->dx = 0;
                pActor->dy = -1;     
                pActor->ghostDead = 0;
            }
        }
        else
        {
            targetx = actor_Player.x;
            targety = actor_Player.y;
        }
          
        pActor->x += pActor->dx;
        pActor->y += pActor->dy;

        // Deal with the tunnel
        if (pActor->x < 21) pActor->x = 237;
        if (pActor->x > 237) pActor->x = 21;        

        if (pActor->dx != 0)
        {        
            screenx = spritexToscreenx (pActor->x);
            screeny = spriteyToscreeny (pActor->y);
            if (screenxTospritex(screenx) == pActor->x)
            {
                isItBlocked = isBlockedGhostDoorOpen(screenx + pActor->dx, screeny );                
                if (isItBlocked)
                {
                    // Stop X motion
                    pActor->dx = 0;                                                              

                    // The player is above us
                    if ((aggressivey == 1  && targety < pActor->y) || (aggressivey == 0 && targety > pActor->y ))
                    {
                        // Go Up                    
                        if (!isBlocked(screenx, screeny - 1 ))
                        {
                            pActor->dy = -1;                            
                        }
                        // If that's blocked go the other way
                        else if (!isBlocked(screenx, screeny + 1 ))
                        {
                            pActor->dy = 1;                               
                        }                                                          
                    }

                    // The player is below us
                    else
                    {
                        // Go Down                    
                        if (!isBlocked(screenx, screeny + 1 ))
                        {
                            pActor->dy = 1;                        
                        }
                        // If that's blocked go the other way                
                        else if (!isBlocked(screenx, screeny - 1 ))
                        {
                            pActor->dy = -1;                            
                        }                                 
                    }                
                }  
                else
                {
                    if (aggressivex)
                    {
                        // Look up and down to see if we can get closer to  player                
                        if (!isBlocked(screenx, screeny - 1) && targety < pActor->y)
                        {
                            pActor->dy = -1;                    
                            pActor->dx = 0;                                            
                        } 
                        else if (!isBlocked(screenx, screeny + 1) && targety > pActor->y)
                        {
                            pActor->dy = 1;                    
                            pActor->dx = 0;                                             
                        }                 
                    }
                }                                  
            }                    
        }

        else if (pActor->dy != 0)
        {        
            screenx = spritexToscreenx (pActor->x);
            screeny = spriteyToscreeny (pActor->y);    
                        
            if (screenyTospritey(screeny) == pActor->y)
            {
                // If 
                //      moving up
                //      the actor's "GhostDoorOpen" counter is 0;                                
                
                if ((pActor->ghostDoorOpen == 0))
                {
                    if (pActor->dy == 0xff)
                    {                           
                        isItBlocked = isBlockedGhostDoorOpen(screenx, screeny + pActor->dy );                        
                    }
                    else
                    {                        
                        isItBlocked = isBlocked(screenx, screeny + pActor->dy );
                    }
                    
                }
                else
                {                    
                    isItBlocked = isBlocked(screenx, screeny + pActor->dy );                    
                }                            

                if (isItBlocked)
                {
                    // Stop Y motion
                    pActor->dy = 0;

                    // The player is to our left
                    if ((aggressivex == 1  && targetx  < pActor->x) || (aggressivex == 0 && targetx  > pActor->x ))
                    {
                        // Go left                    
                        if (!isBlockedGhostDoorOpen(screenx - 1, screeny ))
                        {
                            pActor->dx = -1;                             
                        }
                        // If that's blocked go the other way
                        else if (!isBlockedGhostDoorOpen(screenx + 1, screeny ))
                        {
                            pActor->dx = 1;                                   
                        }                                      
                    }

                    // The player is to our right
                    else
                    {       
                        
                        if (!isBlockedGhostDoorOpen(screenx + 1, screeny ) )
                        {
                            pActor->dx = 1;                                  
                        }
                        // If that's blocked go the other way                
                        else if (!isBlockedGhostDoorOpen(screenx -1 , screeny))
                        {
                            pActor->dx = -1;                                  
                        }                            
                    }
                }
                else
                {
                    if (aggressivey)
                    {
                        // Look left and right to see if we can get closer to player                
                        if (!isBlockedGhostDoorOpen(screenx - 1, screeny) && targetx  < pActor->x)
                        {
                            pActor->dy = 0;                    
                            pActor->dx = -1;                                 
                        } 
                        else if (!isBlockedGhostDoorOpen(screenx + 1, screeny) && targetx  > pActor->x)
                        {
                            pActor->dy = 0;                    
                            pActor->dx = 1;                                   
                        }
                    }                 
                }            
            }            
        }   
    }

    if (pActor->ghostDead > 1)                                      
    {
        --(pActor->ghostDead);    
        if (pActor->nextGhostScore == 200) pActor->framedata = (char *)&animation_ghost_200;
        if (pActor->nextGhostScore == 400) pActor->framedata = (char *)&animation_ghost_400;
        if (pActor->nextGhostScore == 800) pActor->framedata = (char *)&animation_ghost_800;
        if (pActor->nextGhostScore == 1600) pActor->framedata = (char *)&animation_ghost_1600;
        VIC.spr_color[pActor->spriteNumber] = COLOR_WHITE;
        pActor->multicolor = 0;
    } 
    else if (pActor->ghostDead > 0)
    {                                       
        pActor->multicolor = 1;
        pActor->framedata = (char *)&animation_ghost_dead;            
    }
    else if (pActor->ghostScared > 0)
    {
        pActor->multicolor = 1;
        if (interruptCounter == 0){
            --(pActor->ghostScared);                        
        }                        
        if (pActor->ghostScared >=3)
        {
            VIC.spr_color[pActor->spriteNumber] = COLOR_BLUE;                
            pActor->framedata = (char *)&animation_ghost_scared;
        }
        else
        {
            pActor->framedata = (char *)&animation_ghost_scared;
            if ((interruptCounter & 0x10) == 0x10)
            {
                VIC.spr_color[pActor->spriteNumber] = COLOR_BLUE;                
            }
            else
            {
                VIC.spr_color[pActor->spriteNumber] = COLOR_WHITE;                
            }
        }
    }
    else
    {
        pActor->framedata = LookTowardPlayer(pActor->x, pActor-> y);
        VIC.spr_color[pActor->spriteNumber] = pActor->normalColor;
        pActor->multicolor = 1;
    }                    
    
    if (pActor->moveDelay == 0) {
        if (pActor->ghostScared)
        {
            pActor->moveDelay = (pActor->moveDelayMax / 4);
        }
        else
        {        
            pActor->moveDelay = pActor->moveDelayMax;
        }
    }
}

// Move the actor in the selected direction unless blocked
void fastcall moveActorPlayer()
{
    static unsigned char screenx;
    static unsigned char screeny;  
    static unsigned char xaligned;
    static unsigned char yaligned;

    if (playerDied) return;
        
    if (actor_Player.moveDelayMax == 255 || --(actor_Player. moveDelay) != 1)
    {

        // Act upon the last valid movement
        actor_Player.x += actor_Player.dx;
        actor_Player.y += actor_Player.dy;  

        if (actor_Player.x < 21) actor_Player.x = 237;
        if (actor_Player.x > 237) actor_Player.x = 21;        

        // What are the screen coords?
        screenx = spritexToscreenx (actor_Player.x);
        screeny = spriteyToscreeny (actor_Player.y);    

        // Are we aligned
        xaligned = (screenxTospritex(screenx) == actor_Player.x);
        yaligned = (screenyTospritey(screeny) == actor_Player.y);    

        // Make sure we didn't hit a wall in the X direction
        if ((actor_Player.dx != 0) && xaligned)
        {                    
            if (isBlockedGhostDoorOpen(screenx + actor_Player.dx, screeny ))
            {
                // Stop and wait for the player to tell you what to do next
                actor_Player.dx = 0;                
                actor_Player.framedata = (char*)&animation_player_still;  
            }                                                          
        }

        // Make sure we didn't hit a wall in the Y direction
        else if ((actor_Player.dy != 0) && yaligned)
        {                        
            if (isBlocked(screenx, screeny + actor_Player.dy ))
            {
                // Stop and wait for the player to tell you what to do next
                actor_Player.dy = 0;
                actor_Player.framedata = (char*)&animation_player_still;  
            }                                                  
        }    

        // Only accept joystick input when fully alligned
        if (xaligned && yaligned)
        {
            // Joystick left
            if (!(CIA1.pra & 4) && (!isBlocked(screenx - 1, screeny)))
            {
                actor_Player.dy = 0;
                actor_Player.dx = -1;
                actor_Player.framedata = (char*)&animation_player_left;  
            }
            // Joystick right
            else if (!(CIA1.pra & 8) && (!isBlocked(screenx + 1, screeny)))
            {
                actor_Player.dy = 0;
                actor_Player.dx = 1;
                actor_Player.framedata = (char*)&animation_player_right;  
            }

            // Joystick up
            else if (!(CIA1.pra & 1) && (!isBlocked(screenx, screeny - 1 )))
            {
                actor_Player.dy = -1;
                actor_Player.dx = 0;
                actor_Player.framedata = (char*)&animation_player_up;  
            }
            // Joystick down
            else if (!(CIA1.pra & 2) && (!isBlocked(screenx, screeny + 1)))
            {
                actor_Player.dy = 1;
                actor_Player.dx = 0;
                actor_Player.framedata = (char*)&animation_player_down;  
            }
        }
    }
    
    if (actor_Player.moveDelay == 0) actor_Player.moveDelay = actor_Player.moveDelayMax;
}

#define GRACE 8
unsigned char validateHit(register struct actor *pActor)
{
    static unsigned char ghostx;
    static unsigned char ghosty;
    static unsigned char playerx;
    static unsigned char playery;
    
    ghostx = pActor->x;
    ghosty = pActor->y;
    playerx = actor_Player.x;
    playery = actor_Player.y;
    
    return (playerx >= ghostx - GRACE && playerx <= ghostx + GRACE && playery >= ghosty - GRACE && playery <= ghosty + GRACE);    
}

void fastcall checkActorPlayer()
{
    static unsigned char screenx;
    static unsigned char screeny;  
    static unsigned int address;
    static unsigned char isDead;
    static unsigned char spriteColl;
    
    // What are the screen coords?
    screenx = spritexToscreenx (actor_Player.x);
    screeny = spriteyToscreeny (actor_Player.y);    
    
    if ((screenxTospritex(screenx) == actor_Player.x) && (screenyTospritey(screeny) == actor_Player.y))
    {
        address = screenxyToAddress(screenx, screeny);
        
        // Eat a dot
        if (screenData[address] == 0x2E)
        {   
            score1 += 10;
            --dotsRemaining;
            screenData[address] = 0x20;   
            nextSound1 = (char*)&sfx1_eatDot;
        }

        // Eat a power pill
        if (screenData[address] == 0x51)
        {
            nextSound2 = (char*)&sfx1_eatPill;

            score1 += 100;
            --pillsRemaining;
            screenData[address] = 0x20; 
            
            score1 += nextGhostScore;
            nextGhostScore = 100;

            // Put Ghosts in scared mode
            actor_Ghost1.ghostScared = MAX_GHOST_SCARED;
            actor_Ghost2.ghostScared = MAX_GHOST_SCARED;
            actor_Ghost3.ghostScared = MAX_GHOST_SCARED;
            actor_Ghost4.ghostScared = MAX_GHOST_SCARED;                        
        }
    }

    // Hit ghost (sprite 0 - 3)
    isDead = 0;
    spriteColl = VIC.spr_coll;
    #ifdef DEBUG    
        hitRegister = spriteColl;
    #endif
                    
    //if ((spriteColl & 0x11) == 0x11 && validateHit(&actor_Ghost1)) 
    if (validateHit(&actor_Ghost1)) 
    {
        #ifdef DEBUG    
            hitGhost1 = 1;
        #endif
        if (actor_Ghost1.ghostDead)
        {

        }
        else if (actor_Ghost1.ghostScared > 0)
        {
            actor_Ghost1.ghostScared = 0;
            actor_Ghost1.ghostDead = 60;
            nextGhostScore *= 2;
            actor_Ghost1.nextGhostScore = nextGhostScore;
            score1 += nextGhostScore;  
            nextSound2 = (char*)&sfx1_ghostDie;          
        }
        else
        {
            isDead = 1;            
        }                        
    }
    if (validateHit(&actor_Ghost2)) 
    {
        #ifdef DEBUG    
            hitGhost1 = 2;
        #endif
        if (actor_Ghost2.ghostDead)
        {

        }
        else if (actor_Ghost2.ghostScared > 0)
        {
            actor_Ghost2.ghostScared = 0;
            actor_Ghost2.ghostDead = 60;
            nextGhostScore *= 2;
            actor_Ghost2.nextGhostScore = nextGhostScore;
            score1 += nextGhostScore;            
            nextSound2 = (char*)&sfx1_ghostDie;          
        }
        else
        {
            isDead = 1;
        }                        
    }
    if (validateHit(&actor_Ghost3)) 
    {
        #ifdef DEBUG    
            hitGhost1 = 3;
        #endif     
        if (actor_Ghost3.ghostDead)
        {

        }
        else if (actor_Ghost3.ghostScared > 0)
        {
            actor_Ghost3.ghostScared = 0;
            actor_Ghost3.ghostDead = 60;
            nextGhostScore *= 2;
            actor_Ghost3.nextGhostScore = nextGhostScore;
            score1 += nextGhostScore;            
            nextSound2 = (char*)&sfx1_ghostDie;          
        }
        else
        {
            isDead = 1;
        }                        
    }
    if (validateHit(&actor_Ghost4)) 
    {
        #ifdef DEBUG    
            hitGhost4 = 1;
        #endif
        if (actor_Ghost4.ghostDead)
        {
            
        }
        else if (actor_Ghost4.ghostScared > 0)
        {
            actor_Ghost4.ghostScared = 0;
            actor_Ghost4.ghostDead = 60;
            nextGhostScore *= 2;
            actor_Ghost4.nextGhostScore = nextGhostScore;
            score1 += nextGhostScore;            
            nextSound2 = (char*)&sfx1_ghostDie;          
        }
        else
        {
            isDead = 1;
        }                        
    }        

    if (isDead) 
    {
        // Hit well ghost
        playerDied = 1;            
        actor_Player.framedata = (char*)&animation_player_die;  
        actor_Player.frames = PLAYER_DYING_FRAMES;
        actor_Player.dx = 0;
        actor_Player.dy = 0;            
    }
                    
    // Hit fruit (sprite 5)
    else if ((VIC.spr_coll & 0x30) >= 0x10) 
    {                
        //VIC.bordercolor = COLOR_PURPLE;
    }                
}

