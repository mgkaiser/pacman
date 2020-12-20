extern char *screenData;
extern char *tileData;
extern char *colorData;
extern char *spriteData;
extern char *spriteSlot;
extern unsigned int times40[];
extern unsigned int times8[];

//extern unsigned int  screenxyToAddress(unsigned char screenx, unsigned char screeny);
extern void copyScreen(unsigned int screenDataLen, char* scrnData, char* colData);
extern void copyChars(unsigned int charDataLen, char* charData);
extern void copySprites(unsigned int sprDataLen, char* sprData);
extern void draw_string_char(unsigned char x, unsigned char y, unsigned char ch);
extern void draw_string(unsigned char x, unsigned char y, unsigned char w, char *ch);

#define screenxyToAddress(screenx, screeny) (times40[screeny] + screenx)
#define spriteyToscreeny(spritey) ((spritey - 47) / 8)
#define screenyTospritey(screeny) (times8[screeny] + 47)
#define spritexToscreenx(spritex) ((spritex - 21) / 8)
#define screenxTospritex(screenx) (times8[screenx] + 21)

