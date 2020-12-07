#include <c64.h>
#include "actors.h"
#include "pacman.h"

unsigned char animation_ghost_left_up[]     = { 0x80, 0x84 };
unsigned char animation_ghost_right_up[]    = { 0x81, 0x85 };
unsigned char animation_ghost_left_down[]   = { 0x82, 0x86 };
unsigned char animation_ghost_right_down[]  = { 0x83, 0x87 };

unsigned char animation_player_right[]      = { 0x88, 0x89, 0x8a, 0x89 };
unsigned char animation_player_down[]       = { 0x88, 0x8d, 0x8e, 0x8d };
unsigned char animation_player_up[]         = { 0x88, 0x8b, 0x8c, 0x8b };
unsigned char animation_player_left[]       = { 0x88, 0x8f, 0x90, 0x8f };
unsigned char animation_player_still[]      = { 0x88, 0x88, 0x88, 0x88 };

struct actor actor_Ghost1;
struct actor actor_Ghost2;
struct actor actor_Ghost3;
struct actor actor_Ghost4;
struct actor actor_Player;

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
unsigned int screenxyToAddress(unsigned char screenx, unsigned char screeny)
{
    return (screeny * 40) + screenx;
}

// Are we blocked at the specified coords
unsigned char isBlocked(unsigned char screenx, unsigned char screeny)
{
    unsigned char charToCheck;
    charToCheck = screenData[screenxyToAddress(screenx, screeny)];
    if (charToCheck == 0x2E) return 0;
    if (charToCheck == 0x51) return 0;
    if (charToCheck == 0x20) return 0;
    if (charToCheck == 0x79 && ghostDoorOpen != 0) return 0;
    return 1;
}

// Take the data from the struct and actually draw the actor
void renderActor(struct actor *pActor)
{   
    int spriteNumber;
    
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

// Make the ghosts look at the player
unsigned char *LookTowardPlayer(unsigned char x, unsigned char y)
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

// Move the actor in the selected direction unless blocked
void moveActorGhost(struct actor *pActor, unsigned char aggressivex, unsigned char aggressivey)
{
    unsigned char screenx;
    unsigned char screeny;    

    pActor->x += pActor->dx;
    pActor->y += pActor->dy;

    if (pActor->dx != 0)
    {        
        screenx = spritexToscreenx (pActor->x);
        screeny = spriteyToscreeny (pActor->y);
        if (screenxTospritex(screenx) == pActor->x)
        {
            if (isBlocked(screenx + pActor->dx, screeny ))
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
                if (aggressivex && ghostDoorOpen==0)
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
            if (isBlocked(screenx, screeny + pActor->dy ))
            {
                // Stop Y motion
                pActor->dy = 0;

                // The player is to our left
                if ((aggressivex == 1  && actor_Player.x < pActor->x) || (aggressivex == 0 && actor_Player.x > pActor->x ))
                {
                    // Go left                    
                    if (!isBlocked(screenx - 1, screeny ))
                    {
                        pActor->dx = -1;                        
                    }
                    // If that's blocked go the other way
                    else if (!isBlocked(screenx + 1, screeny ))
                    {
                        pActor->dx = 1;                                                         
                    }                                      
                }

                // The player is to our right
                else
                {       
                    
                    if (!isBlocked(screenx + 1, screeny ) )
                    {
                        pActor->dx = 1;                        
                    }
                    // If that's blocked go the other way                
                    else if (!isBlocked(screenx -1 , screeny))
                    {
                        pActor->dx = -1;                        
                    }                            
                }
            }
            else
            {
                if (aggressivey && ghostDoorOpen==0)
                {
                    // Look left and right to see if we can get closer to player                
                    if (!isBlocked(screenx - 1, screeny) && actor_Player.x < pActor->x)
                    {
                        pActor->dy = 0;                    
                        pActor->dx = -1;                    
                    } 
                    else if (!isBlocked(screenx + 1, screeny) && actor_Player.x > pActor->x)
                    {
                        pActor->dy = 0;                    
                        pActor->dx = 1;                    
                    }
                }                 
            }            
        }            
    }    
    
    pActor->framedata = LookTowardPlayer(pActor->x, pActor-> y);
}

// Move the actor in the selected direction unless blocked
void moveActorPlayer(struct actor *pActor)
{
    unsigned char screenx;
    unsigned char screeny;    

    if (pActor->dx != 0)
    {        
        screenx = spritexToscreenx (pActor->x);
        screeny = spriteyToscreeny (pActor->y);
        if (screenxTospritex(screenx) == pActor->x)
        {
            if (isBlocked(screenx + pActor->dx, screeny ))
            {
                // Stop and wait for the player to tell you what to do next
                if (pActor->dx == 1) 
                {
                    pActor->dx = -1;                
                    pActor->framedata = (char*)&animation_player_left;  
                }
                else
                {
                    pActor->dx = 1 ;
                    pActor->framedata = (char*)&animation_player_right;                      
                }
            }
            else
            {                
                pActor->x += pActor->dx;
            }            
        }    
        else
        {
            pActor->x += pActor->dx;
        }
        
    }

    if (pActor->dy != 0)
    {        
        screenx = spritexToscreenx (pActor->x);
        screeny = spriteyToscreeny (pActor->y);    
        if (screenyTospritey(screeny) == pActor->y)
        {
            if (isBlocked(screenx, screeny + pActor->dy ))
            {
                // Stop and wait for the player to tell you what to do next
                pActor->dy = 0;
                pActor->framedata = (char*)&animation_player_still;  
            }
            else
            {                
                pActor->y += pActor->dy;    
            }            
        }    
        else
        {
            pActor->y += pActor->dy;    
        }
    }    
}

// See if the actor should change direction
void checkActorGhost(struct actor *pActor, unsigned char aggressivex, unsigned char aggressivey)
{
    unsigned char screenx;
    unsigned char screeny;    

    if (pActor->dy != 0)
    {        
        screenx = spritexToscreenx (pActor->x);
        screeny = spriteyToscreeny (pActor->y);
        
        if (!isBlocked(screenx, screeny + pActor->dy))    
        {
            if (screenxTospritex(screenx) == pActor->x)
        {
            // Player is to the left
            if (actor_Player.x < pActor->x) 
            {
                if (aggressivex)
                {
                    // See if we can make a left turn
                    if (!isBlocked(screenx - 1, screeny))
                    {
                        pActor->dy = 0;
                        pActor->dx = -1;
                    } 
                }
                else
                {
                    // See if we can make a right turn
                    if (!isBlocked(screenx + 1, screeny))
                    {
                        pActor->dy = 0;
                        pActor->dx = 1;
                    } 
                }
                
            }
            // Player is to the right
            else
            {
                if (aggressivex)
                {
                    // See if we can make a right turn
                    if (!isBlocked(screenx + 1, screeny))
                    {
                        pActor->dy = 0;
                        pActor->dx = 1;
                    } 
                }
                else
                {
                    // See if we can make a left turn
                    if (!isBlocked(screenx - 1, screeny))
                    {
                        pActor->dy = 0;
                        pActor->dx = -1;
                    } 
                }
            }                        
        }            
        }
    }

    if (pActor->dx != 0)
    {        
        screenx = spritexToscreenx (pActor->x);
        screeny = spriteyToscreeny (pActor->y);
        if (!isBlocked(screenx + pActor->dx, screeny))    
        {
            if (screenyTospritey(screeny) == pActor->y)
        {
            // Player is above us
            if (actor_Player.y < pActor->y) 
            {
                if (aggressivey)
                {
                    // See if we can go up                    
                    if (!isBlocked(screenx, screeny - 1))
                    {
                        pActor->dy = -1;
                        pActor->dx = 0;
                    } 
                }
                else
                {
                    // See if we can go down                    
                    if (!isBlocked(screenx, screeny + 1))
                    {
                        pActor->dy = 1;
                        pActor->dx = 0;
                    } 
                }                                
            }

            // Player is below us
            else
            {
                if (aggressivey)
                {                    
                    // See if we can go down                    
                    if (!isBlocked(screenx, screeny + 1))
                    {
                        pActor->dy = 1;
                        pActor->dx = 0;
                    }  
                }
                else
                {
                    // See if we can go up                    
                    if (!isBlocked(screenx, screeny - 1))
                    {
                        pActor->dy = -1;
                        pActor->dx = 0;
                    }   
                }                    
            }
        }
        }
    }
}

// Did we eat a dot?
// Did we eat a power pill?
// Did we hit a ghost?
// Did we eat fruit?
void checkActorPlayer(struct actor *pActor)
{

}

