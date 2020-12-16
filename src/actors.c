#include <6502.h>
#include <c64.h>
#include <stdio.h>
#include <stdlib.h>
#include "actors.h"
#include "pacman.h"

// Describes the ghost animations
unsigned char animation_ghost_left_up[]     = { 0x80, 0x84 };
unsigned char animation_ghost_right_up[]    = { 0x81, 0x85 };
unsigned char animation_ghost_left_down[]   = { 0x82, 0x86 };
unsigned char animation_ghost_right_down[]  = { 0x83, 0x87 };
unsigned char animation_ghost_scared[]      = { 0x91, 0x91 };

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

// Are we blocked at the specified coords
unsigned char isBlocked(unsigned char screenx, unsigned char screeny)
{
    unsigned char charToCheck;
    charToCheck = screenData[screenxyToAddress(screenx, screeny)];        
    //draw_string_char(30, 10, charToCheck);
    if (charToCheck == 0x2E) return 0;
    if (charToCheck == 0x51) return 0;
    if (charToCheck == 0x20) return 0;     
    return 1;
}

unsigned char isBlockedGhostDoorOpen(unsigned char screenx, unsigned char screeny)
{
    unsigned char charToCheck;
    charToCheck = screenData[screenxyToAddress(screenx, screeny)];    
    //draw_string_char(30, 10, charToCheck);
    if (charToCheck == 0x2E) return 0;
    if (charToCheck == 0x51) return 0;
    if (charToCheck == 0x20) return 0;
    if (charToCheck == 0x79) return 0;    
    return 1;
}

// Make the ghosts look at the player
unsigned char*  LookTowardPlayer(unsigned char x, unsigned char y)
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
void  renderActor(register struct actor *pActor)
{   
    static unsigned char spriteNumber;
    
    spriteNumber = pActor->spriteNumber;

    VIC.spr_pos[spriteNumber].x = pActor->x;
    VIC.spr_pos[spriteNumber].y = pActor->y;

    if (pActor->animationDelay-- == 0)
    {
        spriteSlot[spriteNumber] = *(pActor->framedata + pActor->frame);              
        pActor->frame++;
        if (pActor->frame >= pActor->frames) pActor->frame = 0;
        pActor->animationDelay = pActor->animationDelayMax;
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

// Move the actor in the selected direction unless blocked
void moveActorGhost(register struct actor *pActor)
{
    static unsigned char screenx;
    static unsigned char screeny;  
    static unsigned char aggressivex;
    static unsigned char aggressivey;

    if (playerDied) return;

    if (pActor->ghostDoorOpen != 0) {
        pActor->ghostDoorOpen = pActor->ghostDoorOpen - 1;
        pActor->suppressAggression = 150;
    }
    if (pActor->suppressAggression != 0) pActor->suppressAggression = pActor->suppressAggression - 1;    
        
    if (pActor->moveDelayMax == 255 || --(pActor -> moveDelay) != 1)
    {
        static unsigned char _isBlocked;

        if (pActor->suppressAggression != 0 || pActor->ghostScared > 0)
        {
            aggressivex = 0;
            aggressivey = 0;
        }
        else
        {
            aggressivex = pActor->aggressivex;
            aggressivey = pActor->aggressivey;
        }
  
        pActor->x += pActor->dx;
        pActor->y += pActor->dy;

        if (pActor->x < 21) pActor->x = 237;
        if (pActor->x > 237) pActor->x = 21;        

        if (pActor->dx != 0)
        {        
            screenx = spritexToscreenx (pActor->x);
            screeny = spriteyToscreeny (pActor->y);
            if (screenxTospritex(screenx) == pActor->x)
            {
                _isBlocked = isBlockedGhostDoorOpen(screenx + pActor->dx, screeny );                
                if (_isBlocked)
                {
                    // Stop X motion
                    pActor->dx = 0;                                                              

                    // The player is above us
                    if ((aggressivey == 1  && actor_Player.y < pActor->y) || (aggressivey == 0 && actor_Player.y > pActor->y ))
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
                        if (!isBlocked(screenx, screeny - 1) && actor_Player.y < pActor->y)
                        {
                            pActor->dy = -1;                    
                            pActor->dx = 0;                                            
                        } 
                        else if (!isBlocked(screenx, screeny + 1) && actor_Player.y > pActor->y)
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
                        _isBlocked = isBlockedGhostDoorOpen(screenx, screeny + pActor->dy );                        
                    }
                    else
                    {                        
                        _isBlocked = isBlocked(screenx, screeny + pActor->dy );
                    }
                    
                }
                else
                {                    
                    _isBlocked = isBlocked(screenx, screeny + pActor->dy );                    
                }                            

                if (_isBlocked)
                {
                    // Stop Y motion
                    pActor->dy = 0;

                    // The player is to our left
                    if ((aggressivex == 1  && actor_Player.x < pActor->x) || (aggressivex == 0 && actor_Player.x > pActor->x ))
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
                        if (!isBlockedGhostDoorOpen(screenx - 1, screeny) && actor_Player.x < pActor->x)
                        {
                            pActor->dy = 0;                    
                            pActor->dx = -1;                                 
                        } 
                        else if (!isBlockedGhostDoorOpen(screenx + 1, screeny) && actor_Player.x > pActor->x)
                        {
                            pActor->dy = 0;                    
                            pActor->dx = 1;                                   
                        }
                    }                 
                }            
            }            
        }    
        
        if (pActor->ghostScared > 0)
        {
            if (interruptCounter == 0){
                --(pActor->ghostScared);                        
            }

            pActor->framedata = (char *)&animation_ghost_scared;

            if (pActor->ghostScared >=3)
            {
                VIC.spr_color[pActor->spriteNumber] = COLOR_BLUE;                
            }
            else
            {
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
        }
                
    }
    
    if (pActor->moveDelay == 0) pActor->moveDelay = pActor->moveDelayMax;
}

// Move the actor in the selected direction unless blocked
void  moveActorPlayer()
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

unsigned char validateHit(register struct actor *pActor)
{
    unsigned char ghostx;
    unsigned char ghosty;
    unsigned char playerx;
    unsigned char playery;

    const unsigned char grace = 8;

    ghostx = pActor->x;
    ghosty = pActor->y;
    playerx = actor_Player.x;
    playery = actor_Player.y;

    if (playerx >= ghostx - grace && playerx <= ghostx + grace && playery >= ghosty - grace && playery <= ghosty + grace) return 1;

    return 0;    
}


// Did we eat a dot?
// Did we eat a power pill?
// Did we hit a ghost?
// Did we eat fruit?
void  checkActorPlayer()
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
        }

        // Eat a power pill
        if (screenData[address] == 0x51)
        {
            score1 += 100;
            --pillsRemaining;
            screenData[address] = 0x20;  

            // Put Ghosts in scared mode
            actor_Ghost1.ghostScared = 10;
            actor_Ghost2.ghostScared = 10;
            actor_Ghost3.ghostScared = 10;
            actor_Ghost4.ghostScared = 10;                        
        }
    }

    // Hit ghost (sprite 0 - 3)
    isDead = 0;
    spriteColl = VIC.spr_coll;
    hitRegister = spriteColl;
                    
    //if ((spriteColl & 0x11) == 0x11 && validateHit(&actor_Ghost1)) 
    if (validateHit(&actor_Ghost1)) 
    {
        hitGhost1 = 1;
        if (actor_Ghost1.ghostScared > 0)
        {

        }
        else
        {
            isDead = 1;            
        }                        
    }
    if (validateHit(&actor_Ghost2)) 
    {
        hitGhost2 = 1;
        if (actor_Ghost2.ghostScared > 0)
        {
            
        }
        else
        {
            isDead = 1;
        }                        
    }
    if (validateHit(&actor_Ghost3)) 
    {
        hitGhost3 = 1;        
        if (actor_Ghost3.ghostScared > 0)
        {

        }
        else
        {
            isDead = 1;
        }                        
    }
    if (validateHit(&actor_Ghost4)) 
    {
        hitGhost4 = 1;
        if (actor_Ghost4.ghostScared > 0)
        {
            
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
        VIC.bordercolor = COLOR_PURPLE;
    }                
}

