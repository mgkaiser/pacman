extern char *screenData;
extern char *tileData;
extern char *colorData;
extern char *spriteData;
extern char *spriteSlot;
extern unsigned int times40[];
extern unsigned char screenyTospriteyTable[];
extern unsigned char screenxTospritexTable[];
extern unsigned char spriteyToscreenyTable[];
extern unsigned char spritexToscreenxTable[];

//extern unsigned int  screenxyToAddress(unsigned char screenx, unsigned char screeny);
extern void copyScreen(unsigned int screenDataLen, char* scrnData, char* colData);
extern void copyChars(unsigned int charDataLen, char* charData);
extern void copySprites(unsigned int sprDataLen, char* sprData);
extern void draw_string_char(unsigned char x, unsigned char y, unsigned char ch);
extern void draw_string(unsigned char x, unsigned char y, unsigned char w, char *ch);

#define screenxyToAddress(screenx, screeny) (times40[screeny] + screenx)
#define spriteyToscreeny(spritey) (spriteyToscreenyTable[spritey])
#define screenyTospritey(screeny) (screenyTospriteyTable[screeny])
#define spritexToscreenx(spritex) (spritexToscreenxTable[spritex])
#define screenxTospritex(screenx) (screenxTospritexTable[screenx])

