/*naskfunc.asmで作った関数*/

/*io_hltを使うために最初に宣言する*/
extern void _io_hlt(void);
extern void _io_cli(void);
extern void _io_out8(int port,int data);
extern int _io_load_eflags(void);
extern void _io_store_eflags(int eflags);

extern void _init_palette(void);
extern void _set_palette(int start,int end,unsigned char *rgb);

void bookfill8(unsigned char *vram,int xsize,unsigned char c,int x0,int y0,int x1,int y1);
void init_screen(char *vram, int x, int y);
void putfont8(char *vram, int xsize,int x,int y,char c, char *font);
void putfont8_asc(char *vram, int xsize,int x,int y,char c, unsigned char *s);




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


void HariMain(void){
    char *vram;/*vram のメモリの場所を格納する変数*/
    int xsize,ysize;
    struct BOOTINFO *binfo;
    extern char hankaku[4096];
    _init_palette();
    
    binfo=(struct BOOTINFO *)0x0ff0;
    init_screen(binfo->vram,binfo->scrnx,binfo->scrny);
    putfont8_asc(binfo->vram,binfo->scrnx,8,8,COL8_FFFFFF,"ABC 123");
    putfont8_asc(binfo->vram,binfo->scrnx,31,31,COL8_000000,"Har1B0 OS.");
    putfont8_asc(binfo->vram,binfo->scrnx,30,30,COL8_FFFFFF,"Har1B0 OS.");


    for(;;){
        _io_hlt();
    }
}

void _init_palette(void){
    static unsigned char table_rgb[16*3]={
        0x00, 0x00, 0x00, /*黒*/
        0xff, 0x00, 0x00, /*明るい赤*/
        0xff, 0xff, 0x00, /*明るい緑*/
        0xff, 0xff, 0x00, /*明るい黄*/
        0x00, 0x00, 0xff, /*明るい青*/
        0xff, 0x00, 0xff, /*明るい紫*/
        0x00, 0xff, 0xff, /*明るい水色*/
        0xff, 0xff, 0xff, /*白*/
        0xc6, 0xc6, 0xc6, /*明るい灰色*/
        0x84, 0x00, 0x00, /*暗い赤*/
        0x00, 0x84, 0x00, /*暗い緑*/
        0x84, 0x84, 0x00, /*暗い黄*/
        0x00, 0x00, 0x84, /*暗い青*/
        0x84, 0x00, 0x84, /*暗い紫*/
        0x00, 0x84, 0x84, /*暗い水色*/
        0x84, 0x84, 0x84, /*暗い灰色*/
    };
    _set_palette(0,15,table_rgb);
    return ;
}

void _set_palette(int start, int end ,unsigned char *rgb){
    int i,eflags;
    eflags=_io_load_eflags();    /*割り込み許可を数えるフラグ*/
    _io_cli();                   /*フラグを0にする*/
    _io_out8(0x03c8,start);
    for(i=start;i<=end;i++){
        _io_out8(0x03c9,rgb[0]/4);
        _io_out8(0x03c9,rgb[1]/4);
        _io_out8(0x03c9,rgb[2]/4);
        rgb+=3;
    }
    _io_store_eflags(eflags);
    return ;
}

void bookfill8(unsigned char *vram,int xsize,unsigned char c,int x0,int y0,int x1,int y1){
    int x,y;
    for(y=y0;y<=y1;y++){
        for(x=x0;x<=x1;x++){
            vram[y*xsize+x]=c;
        }
    }
    return ;
}

void init_screen(char *vram, int x, int y)
{
	bookfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	bookfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	bookfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	bookfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	bookfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	bookfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	bookfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	bookfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	bookfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	bookfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	bookfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	bookfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	bookfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	bookfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}

void putfont8(char *vram, int xsize,int x,int y,char c, char *font){
    int i;
    char data;
    char *p; /*4byte*/ 
    for(i=0;i<16;i++){
        p=vram+(y+i)*xsize+x;
        data=font[i];
        if((data&0x80)!=0){
            p[0]=c;
        }
        if((data&0x40)!=0){
            p[1]=c;
        }
        if((data&0x20)!=0){
            p[2]=c;
        }
        if((data&0x10)!=0){
            p[3]=c;
        }
        if((data&0x08)!=0){
            p[4]=c;
        }
        if((data&0x04)!=0){
            p[5]=c;
        }
        if((data&0x02)!=0){
            p[6]=c;
        }
        if((data&0x01)!=0){
            p[7]=c;
        }
    }
    return ;
}


void putfont8_asc(char *vram,int xsize,int x,int y,char c,unsigned char *s){
    extern char hankaku[4096];
    //ここで、文字列を一個ずつ読んでいる
    //文字列の最後が0x00なのでそこまで位置をずらしながら書いている
    for(;*s!=0x00;s++){
        putfont8(vram,xsize,x,y,c,hankaku+*s*16);
        x+=8;
    }
    return ;
}

