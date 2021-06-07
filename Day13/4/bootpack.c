#include<stdio.h>
#include"bootpack.h"


void make_window8(unsigned char *buf,int xsize,int ysize,char *title);

void putfonts8_asc_sht(struct SHEET *sht,int x,int y,int c,int b,char *s,int l);

void HariMain(void){
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    char s[40];	/*デバッグに必要な変数*/
	struct FIFO32 fifo32;
	int fifobuf[128];
	struct TIMER *timer,*timer2,*timer3;

    int i,mx,my,count=0;
    struct MOUSE_DEC mdec;

    unsigned int memtotal;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	struct SHTCTL *shtctl;
	struct SHEET *sht_back,*sht_mouse,*sht_win;
	unsigned char *buf_back,buf_mouse[256],*buf_win;
	

    init_gdtidt();
    init_pic();
    _io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
    

    fifo32_init(&fifo32,128,fifobuf);
	init_pit();
    _io_out8(PIC0_IMR, 0xf8); /* PIC1とキーボードを許可(11111000) ここでは後ろから数値が入るつまり（10001111）の順番ではいる */
    _io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111)ここでも(11110111)で入るこれは0x2cに値する */
	
	timer=timer_alloc();
	timer_init(timer,&fifo32,10);
	timer_settime(timer,1000);

	timer2=timer_alloc();
	timer_init(timer2,&fifo32,3);
	timer_settime(timer2,300);

	timer3=timer_alloc();
	timer_init(timer3,&fifo32,1);
	timer_settime(timer3,50);


    init_keyboard(&fifo32,256);
    enable_mouse(&fifo32,512,&mdec);

    memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
    
    _init_palette();
	shtctl=shtctl_init(memman,binfo->vram,binfo->scrnx,binfo->scrny);
	sht_back=sheet_alloc(shtctl);
	sht_mouse=sheet_alloc(shtctl);
	sht_win=sheet_alloc(shtctl);
	buf_back=(unsigned char *)memman_alloc_4k(memman,binfo->scrnx*binfo->scrny);
	buf_win=(unsigned char *)memman_alloc_4k(memman,160*52);
	sheet_setbuf(sht_back,buf_back,binfo->scrnx,binfo->scrny,-1);
	sheet_setbuf(sht_mouse,buf_mouse,16,16,99);
	sheet_setbuf(sht_win,buf_win,160,52,-1);
    init_screen(buf_back,binfo->scrnx,binfo->scrny);	
    init_mouse_cursor8(buf_mouse,99);
	make_window8(buf_win,160,52,"counter");
	sheet_slide(sht_back,0,0);
	mx=(binfo->scrnx-16)/2;
	my=(binfo->scrny-28-16)/2;
	sheet_slide(sht_mouse,mx,my);
	sheet_slide(sht_win,80,72);
	sheet_updown(sht_back,0);
	sheet_updown(sht_win,1);
	sheet_updown(sht_mouse,2);
    sprintf(s,"(%d,%d)",mx,my);
    putfonts8_asc_sht(sht_back,0,0,COL8_FFFFFF,COL8_008484,s,10);
    sprintf(s, "memory %dMB   free : %dKB",memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF,COL8_008484, s,40);
    for(;;){
		count++;
		
        _io_cli(); /*割り込み禁止*/
        if(fifo32_status(&fifo32)== 0){
            _io_sti(); /*割り込み禁止を解除しながらhlt(cpuを待機状態にする)*/
        } else {
            i=fifo32_get(&fifo32);
            _io_sti();/*CPUの割り込み禁止解除*/
            if(256<=i&&i<=511){
                /*keyboard_data*/
                sprintf(s, "%x",i-256);
			    putfonts8_asc_sht(sht_back,0,16,COL8_FFFFFF,COL8_008484,s,2);
            }else if(512<=i&&i<=767){
                /*mosue_data*/
                if(mouse_decode(&mdec,i-512)!=0){
                    sprintf(s,"[lcr %d %d]",mdec.x,mdec.y);
					if((mdec.btn&0x01)!=0){
						s[1]='L';
					}
					if((mdec.btn&0x02)!=0){
						s[3]='R';
					}
					if((mdec.btn&0x04)!=0){
						s[2]='C';
					}
					putfonts8_asc_sht(sht_back,32,16,COL8_FFFFFF,COL8_008484,s,15);
				   	/*マウスカーソルの移動*/
                    mx+=mdec.x;
                    my+=mdec.y;
                    if(mx < 0){
                        mx=0;
                    }
                    if(my < 0){
                        my=0;
                    }
                    if(mx > binfo->scrnx-1){
                        mx=binfo->scrnx-1;
                    }
                    if(my > binfo->scrny-1){
                        my=binfo->scrny-1;
                    }
                    sprintf(s,"(%d,%d)",mx,my);
                    putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF,COL8_008484,s,10); 
					sheet_slide(sht_mouse,mx,my); /* マウス描く */                 
				}    

            }else if(i==10){	
					putfonts8_asc_sht(sht_back,0,64,COL8_FFFFFF,COL8_008484,"10[sec]",7);

					sprintf(s,"%d",count);
					putfonts8_asc_sht(sht_win,40,28,COL8_000000,COL8_C6C6C6,s,10);
			}else if(i==3){
					putfonts8_asc_sht(sht_back,0,80,COL8_FFFFFF,COL8_008484,"3[sec]",6);
					count=0;
           	}else if(i==1){
					timer_init(timer3,&fifo32,0);
					bookfill8(buf_back,binfo->scrnx,COL8_FFFFFF,8,96,15,111);
                    timer_settime(timer3,50);
                    sheet_refresh(sht_back,8,96,16,112);
			}else if(i==0){
                	timer_init(timer3,&fifo32,1);
					bookfill8(buf_back,binfo->scrnx,COL8_008484,8,96,15,111);
					timer_settime(timer3,50);
					sheet_refresh(sht_back,8,96,16,112);
			}
        }
	}
}
void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	bookfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	bookfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	bookfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	bookfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	bookfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	bookfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	bookfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	bookfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
	bookfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	bookfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	putfont8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

void putfonts8_asc_sht(struct SHEET *sht,int x,int y,int c,int b,char *s ,int l){
	bookfill8(sht->buf,sht->bxsize,b,x,y,x+l*8-1,y+15);
	putfont8_asc(sht->buf,sht->bxsize,x,y,c,s);
	sheet_refresh(sht,x,y,x+l*8,y+16);
	return ;
}