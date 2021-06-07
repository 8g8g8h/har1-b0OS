#include<stdio.h>
#include"bootpack.h"


void make_window8(unsigned char *buf,int xsize,int ysize,char *title,char act);
void make_wtitle8(unsigned char *buf,int xsize,char *title,char act);
void putfonts8_asc_sht(struct SHEET *sht,int x,int y,int c,int b,char *s,int l);

void make_textbook8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

void console_task(struct SHEET *sheet,unsigned int memtotal);

int cons_newline(int cursor_y,struct SHEET *sheet);

void HariMain(void){
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    char s[40];	/*デバッグに必要な変数*/
    struct FIFO32 fifo,keycmd;
    int fifobuf[128],keycmd_buf[32];

    int i,mx,my,cursor_x,cursor_c;
    struct MOUSE_DEC mdec;

    unsigned int memtotal;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    
    struct SHTCTL *shtctl;
    struct SHEET *sht_back,*sht_mouse,*sht_win,*sht_cons;
    unsigned char *buf_back,buf_mouse[256],*buf_win,*buf_cons;

    struct TASK *task_a,*task_cons;
    struct TIMER *timer;

    int key_to=0,key_shift=0,key_leds=(binfo->leds>>4)&7;//binfo->ledsのビット4~6だけ知れればいいので右シフトを行って、3ビット取り出している。
    int keycmd_wait=-1;

    static char keytable0[0x80] = {
      0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
    static char keytable1[0x80]={
      0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
    };
    
    init_gdtidt();
    init_pic();
    _io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */

    fifo32_init(&fifo,128,fifobuf,0);
    init_pit();
    
    init_keyboard(&fifo,256);
    enable_mouse(&fifo,512,&mdec);

    _io_out8(PIC0_IMR, 0xf8); /* PIC1とキーボードを許可(11111000) ここでは後ろから数値が入るつまり（10001111）の順番ではいる */

    _io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111)ここでも(11110111)で入るこれは0x2cに値する */

    fifo32_init(&keycmd,32,keycmd_buf,0);

    //メモリについて
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);
    
    _init_palette();
    shtctl=shtctl_init(memman,binfo->vram,binfo->scrnx,binfo->scrny);
    task_a=task_init(memman);
    fifo.task=task_a;
    task_run(task_a,1,2);

    /*sht_back*/
    sht_back=sheet_alloc(shtctl);
    buf_back=(unsigned char *)memman_alloc_4k(memman,binfo->scrnx*binfo->scrny);
    sheet_setbuf(sht_back,buf_back,binfo->scrnx,binfo->scrny,-1);
    init_screen(buf_back,binfo->scrnx,binfo->scrny);
  
    /*sht_cons*/
    sht_cons=sheet_alloc(shtctl);
    buf_cons=(unsigned char *)memman_alloc_4k(memman,256*165);
    sheet_setbuf(sht_cons,buf_cons,256,165,-1);/*透明色なし*/
    make_window8(buf_cons,256,165,"console",0);
    make_textbook8(sht_cons,8,28,240,128,COL8_000000);
    task_cons=task_alloc();
    task_cons->tss.esp=memman_alloc_4k(memman,64*1024)+64*1024-12;
    task_cons->tss.eip=(int)&console_task;
    task_cons->tss.es=1*8;
    task_cons->tss.cs=2*8;
    task_cons->tss.ss=1*8;
    task_cons->tss.ds=1*8;
    task_cons->tss.fs=1*8;
    task_cons->tss.gs=1*8;

    *((int *)(task_cons->tss.esp+4))=(int)sht_cons;
    *((int *)(task_cons->tss.esp+8))=memtotal;
    task_run(task_cons,2,2);/*level=2,priority=2*/    

    /*sht_win*/
    sht_win=sheet_alloc(shtctl);
    buf_win=(unsigned char *)memman_alloc_4k(memman,160*52);
    sheet_setbuf(sht_win,buf_win,160,52,-1);    
    make_window8(buf_win,160,52,"task_a",1);
    make_textbook8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
    cursor_x = 8;
    cursor_c = COL8_FFFFFF;
    timer=timer_alloc();
    timer_init(timer,&fifo,1);
    timer_settime(timer,50);
    
    /*sht_mouse*/
    sht_mouse=sheet_alloc(shtctl);
    sheet_setbuf(sht_mouse,buf_mouse,16,16,99);
    init_mouse_cursor8(buf_mouse,99);
    mx=(binfo->scrnx-16)/2;/*画面中央になるように計算*/
    my=(binfo->scrny-28-16)/2;

    sheet_slide(sht_back,0,0);
    sheet_slide(sht_cons,200,200);
    sheet_slide(sht_win,300,480);
    sheet_slide(sht_mouse,mx,my);
    sheet_updown(sht_back,0);
    sheet_updown(sht_cons,1);
    sheet_updown(sht_win,2);
    sheet_updown(sht_mouse,3);

    /*最初にキーボード状態との食い違いがないように、設定しておく*/
    fifo32_put(&keycmd,KEYCMD_LED);
    fifo32_put(&keycmd,key_leds);
    for(;;){
      if(fifo32_status(&keycmd)>0&&keycmd_wait<0){
        /*キーボードコントローラに送るデータがあれば、送る*/
        keycmd_wait=fifo32_get(&keycmd);
        wait_KBC_sendready();
        _io_out8(PORT_KEYDAT,keycmd_wait);
      }
      _io_cli(); /*割り込み禁止*/
       if(fifo32_status(&fifo)== 0){
         task_sleep(task_a);
         _io_sti();
       } else {
           i=fifo32_get(&fifo);
           _io_sti();/*CPUの割り込み禁止解除*/
           if(256<=i&&i<=511){
               if(i<256+0x80){/*keycode文字*/
                 if(key_shift==0){
                   s[0]=keytable0[i-256];
                 }else{
                   s[0]=keytable1[i-256];
                 }
               }else{
                 s[0]=0;
               }
               if('A'<=s[0]&&s[0]<='Z'){/*入力文字がアルファベット*/
                 if(((key_leds&4)==0&&key_shift==0)||((key_leds&4)!=0&&key_shift!=0)){
                   s[0]+=0x20;/*大文字から小文字に変換*/
                   
                 }
               }
               if(s[0]!=0){
                 if(key_to==0){/*タスクAへ*/
                   if(cursor_x<128){
                     /*一文字表示してから、カーソルを1つすすめる*/
                     s[1]=0;/*ここでnullにすることによりsがこれ以上データを読み込めないことを示している。つまりsの読み込みはs[0](とs[1])だけ*/
                     putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF,s,1);
                     cursor_x+=8;
                   }
                 }else{
                   fifo32_put(&task_cons->fifo,s[0]+256);
                 }
               }
               if(i==256+0x0e){/*バックスペース*/
                 if(key_to==0){
                   if(cursor_x>8){
                     /*カーソルをスペースで消してから、カーソルを1つ戻す*/
                     putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF," ",1);       
                     cursor_x-=8;
                   }
                 }else{/*コンソールへ*/
                     fifo32_put(&task_cons->fifo,8+256);
                   }
                 }
               if(i==256+0x1c){
                if(key_to!=0){
                  fifo32_put(&task_cons->fifo,10+256);
                }
               }
               if(i==256+0x0f){/*tab*/
                 if(key_to==0){
                   key_to=1;
                   make_wtitle8(buf_win,sht_win->bxsize,"task_a",0);
                   make_wtitle8(buf_cons,sht_cons->bxsize,"console",1);
                   cursor_c=-1;/*カーソルを消す*/
                   bookfill8(sht_win->buf,sht_win->bxsize,COL8_FFFFFF,cursor_x,28,cursor_x+7,43);
                   fifo32_put(&task_cons->fifo,2);/*コンソールのカーソルON*/
                 }else{
                   key_to=0;
                   make_wtitle8(buf_win,sht_win->bxsize,"task_a",1);
                   make_wtitle8(buf_cons,sht_cons->bxsize,"console",0);
                   cursor_c=COL8_000000;
                   fifo32_put(&task_cons->fifo,3);/*コンソールのカーソルOFF*/
                 }
                 sheet_refresh(sht_win,0,0,sht_win->bxsize,21);
                 sheet_refresh(sht_cons,0,0,sht_cons->bxsize,21);
               }
               if(i==256+0x2a){/*左シフト ON*/
                 key_shift|=1;
               }
               if(i==256+0x36){/*右シフト　ON*/
                 key_shift|=2;
               }
               if(i==256+0xaa){/*左シフト OFF*/
                 key_shift&=~1;
               }
               if(i==256+0xb6){/*右シフト OFF*/
                 key_shift&=~2;
               }
               if(i==256+0x3a){/*CapsLock*/
                key_leds^=4;
                fifo32_put(&keycmd,KEYCMD_LED);
                fifo32_put(&keycmd,key_leds);
               }
               if(i==256+0x45){/*NumLock*/
                key_leds^=2;
                fifo32_put(&keycmd,KEYCMD_LED);
                fifo32_put(&keycmd,key_leds);
               }
               if(i==256+0x46){/*ScrollLock*/
                key_leds^=1;
                fifo32_put(&keycmd,KEYCMD_LED);
                fifo32_put(&keycmd,key_leds);
               }
               if(i==256+0xfa){/*キーボードがデータを無事に受け取った*/
                keycmd_wait=-1;
               }
               if(i==256+0xfe){/*キーボードがデータを無事に受け取れなかった*/
                wait_KBC_sendready();
                _io_out8(PORT_KEYDAT,keycmd_wait);
               }
               /*カーソル再表示*/
               if(cursor_c>=0){
                bookfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);
               }
               sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);
           }else if(512<=i&&i<=767){
             /*mosue_data*/
             if(mouse_decode(&mdec,i-512)!=0){
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
               timer_init(timer,&fifo,0);
               if(cursor_c>=0){
                cursor_c=COL8_000000;
               }
             }else{
               timer_init(timer,&fifo,1);
               if(cursor_c>=0){
                cursor_c=COL8_FFFFFF;
               }
             }
             timer_settime(timer,50);
             if(cursor_c>=0){
               bookfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);
               sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);
             }
           }
       }
    }
}
void make_wtitle8(unsigned char *buf, int xsize,char *title,char act)
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
	char c,tc,tbc;
  if(act!=0){
    tc=COL8_FFFFFF;
    tbc=COL8_000084;
  }else{
    tc=COL8_C6C6C6;
    tbc=COL8_848484;
  }
  bookfill8(buf,xsize,tbc,3,3,xsize-4,20);
	putfont8_asc(buf, xsize, 24, 4, tc, title);
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

void make_window8(unsigned char *buf,int xsize,int ysize,char *title,char act){
  bookfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	bookfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	bookfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	bookfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	bookfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	bookfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	bookfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
  bookfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	bookfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
  make_wtitle8(buf,xsize,title,act);
  return ;
}

void putfonts8_asc_sht(struct SHEET *sht,int x,int y,int c,int b,char *s ,int l){
	bookfill8(sht->buf,sht->bxsize,b,x,y,x+l*8-1,y+15);
	putfont8_asc(sht->buf,sht->bxsize,x,y,c,s);
	sheet_refresh(sht,x,y,x+l*8,y+16);
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

void console_task(struct SHEET *sheet,unsigned int memtotal){

  struct TIMER *timer;
  int i,fifobuf[128],cursor_x=16,cursor_y=28,cursor_c=-1,x,y;
  struct TASK *task=task_now();
  char s[30],cmdline[30];
  struct MEMMAN *memman=(struct MEMMAN *)MEMMAN_ADDR;

  fifo32_init(&task->fifo,128,fifobuf,task);  
  timer=timer_alloc();
  timer_init(timer,&task->fifo,1);
  timer_settime(timer,50);

  /*コマンドプロンプト表示*/
  putfonts8_asc_sht(sheet,8,28,COL8_FFFFFF,COL8_000000,">",1);

  for(;;){
    _io_cli();
    if(fifo32_status(&task->fifo)==0){
      task_sleep(task);
      _io_sti();
    }else {
      i=fifo32_get(&task->fifo);
      _io_sti();
      if(i<=1){/*カーソル用タイマ*/
        if(i!=0){
          timer_init(timer,&task->fifo,0);/*次は0*/
          if(cursor_c>=0){
            cursor_c=COL8_FFFFFF;
          }
        }else{
          timer_init(timer,&task->fifo,1);/*次は1*/
          if(cursor_c>=0){
            cursor_c=COL8_000000;
          }
        }
        timer_settime(timer,50);
      }
      if(i==2){/*カーソルON*/
        cursor_c=COL8_FFFFFF;
      }
      if(i==3){/*カーソルOFF*/
        bookfill8(sheet->buf,sheet->bxsize,COL8_000000,cursor_x,cursor_y,cursor_x+7,43);
        cursor_c=-1;
      }
      if(256<=i&&i<=511){
        if(i==8+256){
          /*バックスペース*/
          if(cursor_x>16){
            /*カーソルをスペースで消してから、カーソルを1つ戻す*/
            putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000," ",1);
            cursor_x-=8;
          }
        }else if(i==256+10){
          /*Enter */
          /*先に改行を行ってその後にコマンドかどうかを確認している。*/
          putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,    COL8_000000, " ",1);
          cmdline[cursor_x/8-2]=0;/*入力された文字の最後を0にして、文字列の最後を判断している*/
          cursor_y=cons_newline(cursor_y,sheet);/*ここで改行したときに行われる動作を行っている*/

          /*コマンド実行*/
          if(cmdline[0]=='m'&&cmdline[1]=='e'&&cmdline[2]=='m'&&cmdline[3]==0){
            /*memコマンド*/
            sprintf(s,"total  %dMB",memtotal/(1024*1024));
            putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,s,30);
            cursor_y=cons_newline(cursor_y,sheet);
            sprintf(s,"free %dKB",memman_total(memman)/1024);
            putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,s,30);
            cursor_y=cons_newline(cursor_y,sheet);/*free を書いた後の改行*/
            cursor_y=cons_newline(cursor_y,sheet);/*次のコマンドを入力するための改行*/
          }else if(cmdline[0]!=0){
            /*コマンドでもなく、空行でもない*/
            putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,"Bad command.",12);
            cursor_y=cons_newline(cursor_y,sheet);
            cursor_y=cons_newline(cursor_y,sheet);
          }
          /*プロンプト表示*/
          putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,">",1);
          cursor_x=16;
        }else{
          /*一般文字*/
          if(cursor_x<240){
            /*一文字表示してから、カーソルを1つ進める*/
            s[0]=i-256;
            s[1]=0;
            cmdline[cursor_x/8-2]=i-256;
            putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000,s,1);
            cursor_x+=8;
          }
        }
      }
      if(cursor_c>=0){
        bookfill8(sheet->buf,sheet->bxsize,cursor_c,cursor_x,cursor_y,cursor_x+7,cursor_y+15);
      }
      sheet_refresh(sheet,cursor_x,cursor_y,cursor_x+8,cursor_y+16);      
    }
  }
}

int cons_newline(int cursor_y,struct SHEET *sheet){
  int x,y;
  if(cursor_y<28+112){
    cursor_y+=16;/*次の行*/
  }else{
    /*スクロール*/
    for(y=28;y<28+112;y++){
      for(x=8;x<8+240;x++){
        sheet->buf[x+y*sheet->bxsize]=sheet->buf[x+(y+16)*sheet->bxsize];
      }
    }
    for(y=28+112;y<28+128;y++){
      for(x=8;x<8+240;x++){
        sheet->buf[x+y*sheet->bxsize]=COL8_000000;
      }
    }
    sheet_refresh(sheet,8,28,8+240,28+128);
  }
  return cursor_y;
}
