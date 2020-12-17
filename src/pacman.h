#undef DEBUG

extern unsigned char animation_ghost_left_up[];
extern unsigned char animation_ghost_right_up[];
extern unsigned char animation_ghost_left_down[];
extern unsigned char animation_ghost_right_down[];

extern unsigned char animation_player_right[];
extern unsigned char animation_player_down[];
extern unsigned char animation_player_up[];
extern unsigned char animation_player_left[];
extern unsigned char animation_player_still[];

extern unsigned long score1;
extern unsigned long score2;
extern unsigned int dotsRemaining;
extern unsigned int pillsRemaining;
extern unsigned char playerDied;
extern unsigned char interruptCounter;
extern unsigned char ateAPill;

#ifdef DEBUG    
    extern unsigned char hitGhost1;
    extern unsigned char hitGhost2;
    extern unsigned char hitGhost3;
    extern unsigned char hitGhost4;
    extern unsigned char hitRegister;
#endif

#define SID_FILE_OFFSET 0x00