#include<stdio.h>

/*関数を最初に定義する*/


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

void io_hlt(void);

void io_cli(void);

void io_out8(int port, int data);

int io_load_eflags(void);

void io_store_eflags(int eflags);

void init_palette(void);

void set_palette(int start,int end ,unsigned char *rgb);

void bookfill8(unsigned char *vram,int xsize,unsigned char c,int x0,int y0,int x1,int y1);

void writescreen(char *vram,int xsize,int ysize);

void putfont8(char *vram, int xsize,int x,int y,char c,  char *font);

void putfont8_asc(char *vram, int xsize,int x,int y,char c, unsigned char *font);

void init_mouse_cursor8(char *mouse,char bc);

void putblock8_8(char *vram,int vxsize,int pxsize,int pysize,int px0,int py0,char *buf,int bxsize);

struct BOOTINFO{
    char cyls,leds,vmode,reserve;  /*cylsが最初の１バイト(offsetから0)、次のバイトがleds(offsetから1)。といって続いていく*/
    short scrnx,scrny;
    char *vram;
};

struct SEGMENT_DESCRIPTOR {            // 8 byte
  short limit_low, base_low;          // 2 byte * 2
  char base_mid, access_right;        // 1 byte * 2
  char limit_high, base_high;         // 1 byte * 2
};

struct GATE_DESCRIPTOR {               // 10 byte   
  short offset_low, selector;         // 2 byte * 2
	char dw_count, access_right;        // 1 byte * 2
	short offset_high;                  // 2 byte * 2
};

void set_segmdesc(struct SEGMENT_DESCRIPTOR * sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR * gd, int offset, int selector, int ar);
void init_gdtidt(void);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);




void HariMain(void){
	char *vram;/*vram のメモリの場所を格納する変数*/
    int xsize, ysize;
    struct BOOTINFO *binfo;
    extern char hankaku[4096];
    char st[40];/*デバッグのための表示をさせるために必要な変数*/
    char mcursor[256]; /*マウスを表記するための変数*/
    
    
    binfo=(struct BOOTINFO *)0x0ff0;
    xsize=binfo->scrnx;
    ysize=binfo->scrny;   /* グラフィックバッファの開始番地 (cf)asmheadm.asm*/
    vram=binfo->vram; /*番地を代入*/
    int mx=(xsize-16)/2; /*マウスのｘ座標*/
    int my=(ysize-16)/2; /*マウスのｙ座標*/
	
    init_gdtidt();
    init_palette();

    writescreen(vram,xsize,ysize);
    sprintf(st,"(%d,%d)",mx,my);
    putfont8_asc(vram,xsize,8,8,COL8_000000,st);

    init_mouse_cursor8(mcursor,COL8_008484);
    putblock8_8(vram,xsize,16,16,mx,my,mcursor,16);

    for(;;){
       io_hlt();
    }
}

void init_palette(void){
    static unsigned char table_rgb[16*3]={
        0x00, 0x00, 0x00,    /* 0:黒*/
        0xff, 0x00, 0x00,    /* 1:明るい赤*/
        0x00, 0xff, 0x00,    /* 2:明るい緑*/
        0xff, 0xff, 0x00,    /* 3:明るい黄色*/
        0x00, 0x00, 0xff,    /* 4:明るい青*/
        0xff, 0x00, 0xff,    /* 5:明るい紫*/
        0x00, 0xff, 0xff,    /* 6:明るい水色*/
        0xff, 0xff, 0xff,    /* 7:白*/
        0xc6, 0xc6, 0xc6,    /* 8:明るい灰色*/
        0x84, 0x00, 0x00,    /* 9:暗い赤*/
        0x00, 0x84, 0x00,    /* 10:暗い緑*/
        0x84, 0x84, 0x00,    /* 11:暗い黄色*/
        0x00, 0x00, 0x84,    /* 12:暗い青*/
        0x84, 0x00, 0x84,    /* 13:暗い紫*/
        0x00, 0x84, 0x84,    /* 14:暗い水色*/
        0x84, 0x84, 0x84,    /* 15:暗い灰色*/
    };
    set_palette(0,15,table_rgb);
    return ;
}

void set_palette(int start,int end, unsigned char *rgb){
    int i;
    int eflags;
    eflags = io_load_eflags(); /*割り込み許可フラグの値を記録する*/
    io_cli(); 
    io_out8(0x03c8,start);
    for(i=start;i<=end;i++){
        io_out8(0x03c9,rgb[0]/4);
        io_out8(0x03c9,rgb[1]/4);
        io_out8(0x03c9,rgb[2]/4);
        rgb+=3;
    }
    io_store_eflags(eflags); /*割り込み許可フラグを元に戻す*/
    return ;
}

void bookfill8(unsigned char *vram,int xsize,unsigned char c ,int x0,int y0,int x1,int y1){
    int x,y;
    for(y=y0;y<=y1;y++){
        for(x=x0;x<=x1;x++){
            vram[y*xsize+x]=c;
        }
    }
}

void writescreen(char *vram,int xsize,int ysize){
    bookfill8(vram,xsize,COL8_008484,0,0,xsize-1,ysize-29);
    bookfill8(vram,xsize,COL8_C6C6C6,0,ysize-28,xsize-1,ysize-28);
    bookfill8(vram,xsize,COL8_FFFFFF,0,ysize-27,xsize-1,ysize-27);
    bookfill8(vram,xsize,COL8_C6C6C6,0,ysize-26,xsize-1,ysize-1);

    bookfill8(vram,xsize,COL8_FFFFFF,3,ysize-24,59,ysize-24);
    bookfill8(vram,xsize,COL8_FFFFFF,2,ysize-24,2,ysize-4);
    bookfill8(vram,xsize,COL8_848484,3,ysize-4,59,ysize-4);
    bookfill8(vram,xsize,COL8_848484,59,ysize-23,59,ysize-5);
    bookfill8(vram,xsize,COL8_000000,2,ysize-3,59,ysize-3);
    bookfill8(vram,xsize,COL8_000000,60,ysize-24,60,ysize-3);


    bookfill8(vram,xsize,COL8_848484,xsize-47,ysize-24,xsize-4,ysize-24);
    bookfill8(vram,xsize,COL8_848484,xsize-47,ysize-23,xsize-47,ysize-4);
    bookfill8(vram,xsize,COL8_FFFFFF,xsize-47,ysize-3,xsize-4,ysize-3);
    bookfill8(vram,xsize,COL8_FFFFFF,xsize-3,ysize-24,xsize-3,ysize-3);

    return ;
    
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

void putfont8_asc(char *vram, int xsize,int x,int y,char c, unsigned char *font){
    extern char hankaku[4096];
    for(;*font!=0x00;font++){
        putfont8(vram,xsize,x,y,c,hankaku+*font*16);
        x+=8;
    }
    return ;
}

void init_mouse_cursor8(char *mouse,char bc){
    /*マウスカーソルの設定*/
    /*bcは背景色*/
    static char cursor[16][16]={
    "**************..",   // 1
    "*ooooooooooo*...",   // 2
    "*oooooooooo*....",   // 3
    "*ooooooooo*.....",   // 4
    "*oooooooo*......",   // 5
    "*ooooooo*.......",   // 6
    "*ooooooo*.......",   // 7
    "*oooooooo*......",   // 8
    "*oooo**ooo*.....",   // 9
    "*ooo*..*ooo*....",   // 10
    "*oo*....*ooo*...",   // 11
    "*o*......*ooo*..",   // 12
    "**........*ooo*.",   // 13
    "*..........*ooo*",   // 14
    ".. .........*oo*",   // 15
    ".............***" // 12
    };
    int px,py; /* マウスのx,yの位置を格納する変数*/
    for(py=0;py<16;py++){
        for(px=0;px<16;px++){
            if(cursor[px][py]=='*'){
                mouse[py*16+px]=COL8_000000;
            }
            if(cursor[px][py]=='o'){
                mouse[py*16+px]=COL8_FFFFFF;
            }
            if(cursor[px][py]=='.'){
                mouse[py*16+px]=bc;
            }
        }
    }
    return ;
}
void putblock8_8(char *vram,int vxsize,int pxsize,int pysize,int px0,int py0,char *buf,int bxsize){
    int x,y;
    for(y=0;y<pysize;y++){
        for(x=0;x<pxsize;x++){
            vram[(py0+y)*vxsize+(px0+x)]=buf[y*bxsize+x];
        }
    }
    return ;
}

void init_gdtidt(void)
  // global segmentation descriptor table
  // interupt descriptor table
{
  // GDT : 0x00270000-0x0027ffff (65535 byte)
  struct SEGMENT_DESCRIPTOR * gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;   // 0x00270000-0x00270009 (8 byte)
  // GateDescriptor : 0x0026f800-0x0026ffff (28672 byte)
  struct GATE_DESCRIPTOR * idt = (struct GATE_DESCRIPTOR *) 0x0026f800;         // 0x0026f800-0x0026f807 (10 byte)
  int i;

  // initialize GDT
  for (i=0; i < 8192; i++){
    set_segmdesc(gdt + i, 0, 0, 0);
  }
  set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
  set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
  load_gdtr(0xffff, 0x00270000);

  // initialize IDT
  for (i=0; i < 256; i++){
    set_gatedesc(idt + i, 0, 0, 0);
  }
  load_idtr(0x7ff, 0x0026f800);

  return;
}


void set_segmdesc(struct SEGMENT_DESCRIPTOR * sd,  // 4 byte
                  unsigned int limit,             // 4 byte
                  int base,                       // 4 byte
                  int ar                          // 4 byte
    )
{
  if (limit > 0xffff){
    ar |= 0x8000;       // G_bit = 1
    limit /= 0x1000;
  }

  sd->limit_low = limit & 0xffff;
  sd->base_low = base & 0xffff;
  sd->base_mid = (base >> 16) & 0xff;
  sd->access_right = ar & 0xff;
  sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high = (base >> 24) & 0xff;
	return;
}


void set_gatedesc(struct GATE_DESCRIPTOR * gd,
                  int offset,
                  int selector,
                  int ar)
{
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high = (offset >> 16) & 0xffff;
	return;
}


