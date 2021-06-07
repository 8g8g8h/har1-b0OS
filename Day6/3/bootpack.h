/*asmhead.asm*/
struct BOOTINFO{
    char cyls;/*0x0ff0-0x0fff*/
    char leds;/*ブートセクタはどこまでディスクを読んだのか*/
    char vmode;/*ブート時のキーボードのLED状態*/
    char reserve;/*ビデオモード　何ビットカラーか*/
    short scrnx,scrny; /*画像解像度*/
    char *vram;
};

#define ADR_BOOTINFO 0x00000ff0

/*naskfunc.asm*/
void _io_hlt(void);
void _io_cli(void);
void _io_out8(int port,int data);
int _io_load_eflags(void);
void _io_store_eflags(int eflags);
void load_gdtr(int limit,int addr);
void load_idtr(int limit,int addr);


/*graphics.c*/
void _init_palette(void);
void _set_palette(int start,int end,unsigned char *rgb);
void bookfill8(unsigned char *vram,int xsize,unsigned char c,int x0,int y0,int x1,int y1);
void init_screen(char *vram, int x, int y);
void putfont8(char *vram,int xsize,int x,int y,char c,char *font);
void putfont8_asc(char *vram, int xsize,int x,int y,char c, unsigned char *s);
void init_mouse_cursor8(char *mouse,char bc);
void putblock8_8(char *vram,int vxsize,int pxsize,int pysize,int px0,int py0,char *buf,int bxsize);

#define COL8_000000	0
#define COL8_FF0000	1
#define COL8_00FF00	2
#define COL8_FFFF00	3
#define COL8_0000FF	4
#define COL8_FF00FF	5
#define COL8_00FFFF	6
#define COL8_FFFFFF	7
#define COL8_C6C6C6	8
#define COL8_840000	9
#define COL8_008400	10
#define COL8_848400	11
#define COL8_000084	12
#define COL8_840084	13
#define COL8_008484	14
#define COL8_848484	15

/*dsctbl.c*/ 
struct SEGMENT_DESCRIPTOR{
    /*short 2byte char 1byte*/
    short limit_low,base_low;
    char base_mid,access_right;
    char limit_high,base_high;
};

struct GATE_DESCRIPTOR{
    short offset_low,selector;
    char dw_count,access_right;
    short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd,unsigned int limit,int base,int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd,int offset,int selector,int ar);

#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff
#define ADR_BOTPAK 0x00280000
#define LIMIT_BOTPAK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
