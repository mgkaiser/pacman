#define INIT_MUSIC(s)               \
    (   __asm__ ("sei"),            \
        __asm__ ("lda #%b", s),     \
        __asm__ ("jsr $c000"),      \
        __asm__ ("cli"));

#define PLAY_MUSIC()                \
    (   __asm__ ("jsr $c003"));

#define PLAY_SFX(s, c) (            \
        __asm__ ("sei"),            \
        __asm__ ("lda #<%v", s),    \
        __asm__ ("ldy #>%v", s),    \
        __asm__ ("ldx #%b", c),     \
        __asm__ ("jsr $c006"),      \
        __asm__ ("cli"))

extern char *musicData;

extern void copyMusic(unsigned int musicSourceLen, char* musicSource);