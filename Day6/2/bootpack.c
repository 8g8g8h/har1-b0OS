#include<stdio.h>

/*naskfunc.asmで作った関数*/


/*io_hltを使うために最初に宣言する*/
extern void _io_hlt(void);
extern void _io_cli(void);
extern void _io_out8(int port,int data);
extern int _io_load_eflags(void);
extern void _io_store_efalgs(int eflags);

extern void _init_palette(void);
extern void _set_palette(int start,int end,unsigned char *rgb);

void bookfill8(unsigned char *vram,int xsize,unsigned char c,int x0,int y0,int x1,int y1);
void init_screen(char *vram, int x, int y);
void putfont8(char *vram, int xsize,int x,int y,char c, char *font);
void init_mouse_cursor8(char *mouse,char bc);
void putfont8_asc(char *vram, int xsize,int x,int y,char c, unsigned char *s);
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

struct BOOTINFO{
    char cyls,leds,vmode,reserve;
    short scrnx,scrny;
    char *vram;
};

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
void load_gdtr(int limit,int addr);
void load_idtr(int limit,int addr);



void HariMain(void){
    struct BOOTINFO *binfo=(struct BOOTINFO *)0x0ff0;
    //extern char hankaku[4096];/*文字表示に必要な変数*/
    char s[40];	/*デバッグに必要な変数*/
    char mcursor[256]; /*マウスを表記するための変数*//*マウスに必要な変数*/
    int mx=(binfo->scrnx-16)/2; /*マウスのｘ座標*/
    int my=(binfo->scrny-28-16)/2; /*マウスのｙ座標*/

    init_gdtidt();
    _init_palette();
    init_screen(binfo->vram,binfo->scrnx,binfo->scrny);

    init_mouse_cursor8(mcursor,COL8_008484);
    putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,mcursor,16);
    sprintf(s,"(%d,%d)",mx,my);
    putfont8_asc(binfo->vram,binfo->scrnx,0,0,COL8_000000,s);

    for(;;){
        _io_hlt();
    }
}

