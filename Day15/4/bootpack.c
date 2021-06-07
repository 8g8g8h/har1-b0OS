#include<stdio.h>
#include"bootpack.h"


void make_window8(unsigned char *buf,int xsize,int ysize,char *title);

void putfonts8_asc_sht(struct SHEET *sht,int x,int y,int c,int b,char *s,int l);

void set490(struct FIFO32 *fifo,int mode);

void make_textbook8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

//タスク状態セグメント
struct TSS32{
  int backlink,esp0,ss0,esp1,ss1,esp2,ss2,cr3;
  int eip,eflags,eax,ecx,edx,ebx,esp,ebp,esi,edi;
  int es,cs,ss,ds,fs,gs;
  int ldtr,iomap;
};

void task_b_main(void);

void HariMain(void){
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    char s[40];	/*デバッグに必要な変数*/
    struct FIFO32 fifo;
    int fifobuf[128];
    struct TIMER *timer,*timer2,*timer3,*timer_ts;

    int i,mx,my,count=0;
    struct MOUSE_DEC mdec;

    unsigned int memtotal;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    
    struct SHTCTL *shtctl;
    struct SHEET *sht_back,*sht_mouse,*sht_win;
    unsigned char *buf_back,buf_mouse[256],*buf_win;

    static char keytable[0x54] = {
      0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
    
    int cursor_x,cursor_c;

#define AR_TSS32  0x0089

    struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    //TSSの設定
    int task_b_esp;
    struct TSS32 tss_a,tss_b;

    make_textbook8(sht_win,8,28,144,16,COL8_FFFFFF);
    cursor_x=8;
    cursor_c=COL8_FFFFFF;
	

    init_gdtidt();
    init_pic();
    _io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
    

    fifo32_init(&fifo,128,fifobuf);
    init_pit();
    
    init_keyboard(&fifo,256);
    enable_mouse(&fifo,512,&mdec);

    _io_out8(PIC0_IMR, 0xf8); /* PIC1とキーボードを許可(11111000) ここでは後ろから数値が入るつまり（10001111）の順番ではいる */

    _io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111)ここでも(11110111)で入るこれは0x2cに値する */
    set490(&fifo,1);
    timer=timer_alloc();
    timer_init(timer,&fifo,10);
    timer_settime(timer,1000);

    timer2=timer_alloc();
    timer_init(timer2,&fifo,3);
    timer_settime(timer2,300);
 
    timer3=timer_alloc();
    timer_init(timer3,&fifo,1);
    timer_settime(timer3,50);

    //タスクスイッチ用のタイマ
    timer_ts=timer_alloc();
    timer_init(timer_ts,&fifo,2);
    timer_settime(timer_ts,2);

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
    make_window8(buf_win,160,52,"window");

    make_textbook8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
    cursor_x = 8;
    cursor_c = COL8_FFFFFF; 

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

    //TSSの設定(構造体のデータを定義している)
    tss_a.ldtr=0;
    tss_a.iomap=0x40000000;

    tss_b.ldtr=0;
    tss_b.iomap=0x40000000;

    set_segmdesc(gdt+3,103,(int)&tss_a,AR_TSS32);
    set_segmdesc(gdt+4,103,(int)&tss_b,AR_TSS32);
    
    //ここで現在動いているタスクの設定をしている。
    load_tr(3*8);

    //ここでは、タスクBのためのスタックのメモリを確保している
    //なおmemman_alloc_4kは確保したメモリの最初の番地を示している
    task_b_esp=memman_alloc_4k(memman,64*1024)+64*1024;

    //tss_bの初期値設定
    tss_b.eip=(int) &task_b_main;//eipは、このタスクに変わったときの実行してほしいアドレスを示してる。
    tss_b.eflags=0x00000202;/*IF=1*/
    tss_b.eax=0;
    tss_b.ecx=0;
    tss_b.edx=0;
    tss_b.ebx=0;
    tss_b.esp=task_b_esp;//タスクBのためのスタック(なおここで示しているのは、スタックの最終番地を示している)
    tss_b.ebp=0;
    tss_b.esi=0;
    tss_b.edi=0;
    tss_b.es=1*8;
    tss_b.cs=2*8;
    tss_b.ss=1*8;
    tss_b.ds=1*8;
    tss_b.fs=1*8;
    tss_b.gs=1*8;

    *((int *)0x0fec)=(int) sht_back;

    for(;;){
        _io_cli(); /*割り込み禁止*/
        if(fifo32_status(&fifo)== 0){
            _io_stihlt(); /*割り込み禁止を解除しながらhlt(cpuを待機状態にする)*/
        } else {
            i=fifo32_get(&fifo);
            _io_sti();/*CPUの割り込み禁止解除*/
            if(i==2){
              farjmp(0,4*8);
              timer_settime(timer_ts,2);
            }else if(256<=i&&i<=511){
                /*keyboard_data*/
                sprintf(s, "%x",i-256);
                putfonts8_asc_sht(sht_back,0,16,COL8_FFFFFF,COL8_008484,s,2);
                if(i<256+0x54){
                    if(keytable[i-256]!=0&&cursor_x<144){
                        /*一文字表示してから、カーソルを1つすすめる*/
                        s[0]=keytable[i-256];
                        s[1]=0; //ここでnullにすることによりsがこれ以上データを読み込めないことを示している。つまりsの読み込みはs[0](とs[1])だけ
                        putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_C6C6C6,s,1);
                        cursor_x+=8;
                    }
                   
                }
                if(i==256+0x0e&&cursor_x>8){
                    /*カーソルをスペースで消してから、カーソルを1つ戻す*/
                    putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF," ",1);
                    cursor_x-=8;
                }
                bookfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);
                sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);
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
                    if((mdec.btn&0x01)!=0){
                        /*左ボタンを押していたら、sht_winを動かす*/                         sheet_slide(sht_win,mx-80,my-8);
                    }                 
                }    
            }else if(i==10){
              putfonts8_asc_sht(sht_back,0,64,COL8_FFFFFF,COL8_008484,"10[sec]",7);
            }else if(i==3){
              putfonts8_asc_sht(sht_back,0,80,COL8_FFFFFF,COL8_008484,"3[sec]",6);
            }else if(i<=1){
              //カーソル用タイマ
              if(i!=0){
                timer_init(timer3,&fifo,0);
                cursor_c=COL8_000000;
              }else{
                timer_init(timer3,&fifo,1);
                cursor_c=COL8_FFFFFF;
              }
              timer_settime(timer3,50);
              bookfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);
              sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);
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

void set490(struct FIFO32 *fifo,int mode){
    int i;
    struct TIMER *timer;
    if(mode!=0){
        for(i=0;i<490;i++){
            timer=timer_alloc();
            timer_init(timer,fifo,i+1024);
            timer_settime(timer,100*60*60*24*50+i*100);
        }
    }
    return ;
}

void make_textbook8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	bookfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	bookfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	bookfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	bookfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	bookfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	bookfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	bookfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	bookfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	bookfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void task_b_main(void){

  struct FIFO32 fifo;
  struct TIMER *timer_ts;
  struct SHEET *sht_back;
  int i,fifobuf[128];
  char s[11];
  int count=0;

  fifo32_init(&fifo,128,fifobuf);
  timer_ts=timer_alloc();
  timer_init(timer_ts,&fifo,1);
  timer_settime(timer_ts,2);
  
  sht_back=(struct SHEET *)*((int *)0x0fec);

  for(;;){
    count++;
    sprintf(s,"%d",count);
    putfonts8_asc_sht(sht_back,0,144,COL8_FFFFFF,COL8_008484,s,10);
    _io_cli();
    if(fifo32_status(&fifo)==0){
      _io_sti();
    }else {
      i=fifo32_get(&fifo);
      _io_sti();
      if(i==1){
        farjmp(0,3*8);
        timer_settime(timer_ts,2);
      }
    }
  }
}
