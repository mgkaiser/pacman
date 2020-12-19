// Structure to hold data about the actor
struct actor {
    unsigned char spriteNumber;
    unsigned char x;
    unsigned char y;
    unsigned char dx;
    unsigned char dy;
    unsigned char normalColor;
    unsigned char aggressivex;
    unsigned char aggressivey;
    unsigned char animationDelay;
    unsigned char animationDelayMax;
    unsigned char moveDelay;
    unsigned char moveDelayMax;
    unsigned char frame;
    unsigned char frames; 
    unsigned char ghostDoorOpen;              
    unsigned char suppressAggression;
    unsigned char *framedata;
    unsigned char ghostScared;
    unsigned char ghostDead; 
    unsigned char negativeMask;
    unsigned char positiveMask;
    unsigned char multicolor;   
};

#define MAX_GHOST_SCARED 10

#define GHOST_FRAMES 2
extern unsigned char animation_ghost_left_up[];
extern unsigned char animation_ghost_right_up[];
extern unsigned char animation_ghost_left_down[];
extern unsigned char animation_ghost_right_down[];

#define PLAYER_FRAMES 4
extern unsigned char animation_player_right[];
extern unsigned char animation_player_down[];
extern unsigned char animation_player_up[];
extern unsigned char animation_player_left[];
extern unsigned char animation_player_still[];

#define PLAYER_DYING_FRAMES 36
extern unsigned char animation_player_die[];

extern struct actor actor_Ghost1;
extern struct actor actor_Ghost2;
extern struct actor actor_Ghost3;
extern struct actor actor_Ghost4;
extern struct actor actor_Player;

extern void renderActor(register struct actor *Actor);
extern void moveActorGhost(register struct actor *pActor);
extern void moveActorPlayer();
extern void checkActorPlayer();