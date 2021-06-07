#include"bootpack.h"

void _init_palette(void){
    static unsigned char table_rgb[16*3]={
        0x00, 0x00, 0x00, /*黒*/
        0xff, 0x00, 0x00, /*明るい赤*/
        0x00, 0xff, 0x00, /*明るい緑*/
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
    unsigned char table2[216*3];
    int r,g,b;
    _set_palette(0,15,table_rgb);
    for(b=0;b<6;b++){
      for(g=0;g<6;g++){
        for(r=0;r<6;r++){
          table2[(r+g*6+b*36)*3+0]=r*51;
          table2[(r+g*6+b*36)*3+1]=g*51;
          table2[(r+g*6+b*36)*3+2]=b*51;
        }
      }
    }
    _set_palette(16,231,table2);
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
    char *p,data; /*4byte*/ 
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



void putfont8_asc(char *vram, int xsize,int x,int y,char c, unsigned char *s){

   extern char hankaku[4096];
   for(;*s !=0x00;s++){
	putfont8(vram,xsize,x,y,c,hankaku+*s*16);
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
    ".............***" // 16
    };
    int px,py; /* マウスのx,yの位置を格納する変数*/
    for(py=0;py<16;py++){
        for(px=0;px<16;px++){
            if(cursor[py][px]=='*'){
                mouse[py*16+px]=COL8_000000;
            }
            if(cursor[py][px]=='o'){
                mouse[py*16+px]=COL8_FFFFFF;
            }
            if(cursor[py][px]=='.'){
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

