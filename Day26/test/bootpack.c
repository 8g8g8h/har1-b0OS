#include"bootpack.h"

void keywin_off(struct SHEET *key_win);

void keywin_on(struct SHEET *key_win);

struct SHEET *open_console(struct SHTCTL *shtctl,unsigned int memtotal){
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHEET *sht = sheet_alloc(shtctl);
	unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	struct TASK *task = task_alloc();
	int *cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
	sheet_setbuf(sht, buf, 256, 165, -1); /* 透明色なし */
	make_window8(buf, 256, 165, "console", 0);
	make_textbook8(sht, 8, 28, 240, 128, COL8_000000);
	//task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
	task->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;	
	task->tss.eip = (int) &console_task;
	task->tss.es = 1 * 8;
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	*((int *) (task->tss.esp + 4)) = (int) sht;
	*((int *) (task->tss.esp + 8)) = memtotal;
	task_run(task, 2, 2); /* level=2, priority=2 */
	sht->task = task;
	sht->flags |= 0x20;	/* カーソルあり */
	fifo32_init(&task->fifo, 128, cons_fifo, task);
	return sht;
}

void close_constask(struct TASK *task)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	task_sleep(task);
	task->cons_stack=memman_alloc_4k(memman, 64 * 1024);
	memman_free_4k(memman, task->cons_stack, 64 * 1024);
	memman_free_4k(memman, (int) task->fifo.buf, 128 * 4);
	task->flags = 0; /* task_free(task); の代わり */
	return;
}

void close_console(struct SHEET *sht)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task = sht->task;
	memman_free_4k(memman, (int) sht->buf, 256 * 165);
	sheet_free(sht);
	close_constask(task);
	return;
}

void HariMain(void){
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    struct SHTCTL *shtctl;
    char s[40];	/*デバッグに必要な変数*/
    struct FIFO32 fifo,keycmd;
    int fifobuf[128],keycmd_buf[32],*cons_fifo[2];

    int mx,my,i;
    unsigned int memtotal;
    struct MOUSE_DEC mdec;



    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    unsigned char *buf_back,buf_mouse[256],*buf_cons[2];
    struct SHEET *sht_back,*sht_mouse;
    struct TASK *task_a,*task_cons[2],*task;

    static char keytable0[0x80] = {
      0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0x08,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
    static char keytable1[0x80]={
      0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0x08,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0x0a,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
    };
    int key_shift=0,key_leds=(binfo->leds>>4)&7;//binfo->ledsのビット4~6だけ知れればいいので右シフトを行って、3ビット取り出している。
    int keycmd_wait=-1;
    int j,x,y;
    int mmx=-1,mmy=-1,mmx2=0,new_mx=-1,new_my=0,new_wx=0x7fffffff,new_wy=0;
    struct SHEET *sht=0,*key_win;
    
    init_gdtidt();
    init_pic();
    _io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */

    fifo32_init(&fifo,128,fifobuf,0);
    *((int *) 0x0fec) = (int) &fifo;
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
    *((int *)0x0fe4)=(int)shtctl;
    /*sht_back*/
    sht_back=sheet_alloc(shtctl);
    buf_back=(unsigned char *)memman_alloc_4k(memman,binfo->scrnx*binfo->scrny);
    sheet_setbuf(sht_back,buf_back,binfo->scrnx,binfo->scrny,-1);
    init_screen(buf_back,binfo->scrnx,binfo->scrny);
  
    /*sht_cons*/
    key_win=open_console(shtctl,memtotal);
            
    /*sht_mouse*/
    sht_mouse=sheet_alloc(shtctl);
    sheet_setbuf(sht_mouse,buf_mouse,16,16,99);
    init_mouse_cursor8(buf_mouse,99);
    mx=(binfo->scrnx-16)/2;/*画面中央になるように計算*/
    my=(binfo->scrny-28-16)/2;

    sheet_slide(sht_back,0,0);
    sheet_slide(key_win,32,4);
    sheet_slide(sht_mouse,mx,my);
    sheet_updown(sht_back,0);
    sheet_updown(key_win,1);
    sheet_updown(sht_mouse,2); 
    keywin_on(key_win);

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
         //fifoが空になったので、保留している描画があれば実行する
         if(new_mx>=0){
          _io_sti();
          sheet_slide(sht_mouse,new_mx,new_my);
          new_mx=-1;
         }else if(new_wx!=0x7fffffff){
          _io_sti();
          sheet_slide(sht,new_wx,new_wy);
          new_wx=0x7fffffff;
         }else{
          task_sleep(task_a);
          _io_sti();
         }
       } else {
           i=fifo32_get(&fifo);
           _io_sti();/*CPUの割り込み禁止解除*/
           if(key_win->flags==0&&key_win!=0){
            /*入力ウィンドウが閉じられた*/
            if(shtctl->top==1){
             key_win=0;
            }
            key_win=shtctl->sheets[shtctl->top-1];/*次の上のウィンドウを一番上にしている*/
            keywin_on(key_win);
           }
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
               if(s[0]!=0&&key_win!=0){               
                 fifo32_put(&key_win->task->fifo,s[0]+256);
               }
               if(i==256+0x0f&&key_win!=0){/*tab*/
                 keywin_off(key_win);
                 j=key_win->height-1;
                 if(j==0){/*一番下まで来たので、次を一番上にしている*/
                  j=shtctl->top-1;
                 }
                 key_win=shtctl->sheets[j];
                 keywin_on(key_win);
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
               if(i==256+0x3b&&key_shift!=0&&key_win!=0){
                 task=key_win->task;
                 if(task!=0&&task->tss.ss0!=0){
                   /*Shift+F1*/ 
                   cons_putstr0(task->cons,"\nBreak(key) :\n");
                   _io_cli();/*強制終了処理中にタスクが変わると困るから*/
                   task->tss.eax=(int)&(task->tss.esp0);/*ここで、asm_hrb_apiのasm_end_appの準備を行っている。*/
                   task->tss.eip=(int)asm_end_app;/*タスクのeip(次に行う命令のアドレスを示すレジスタ)にasm_end_appを設定することにより、そこに移動する。そのときeaxの値も移動する*/
                   _io_sti();
                 }
               }
               if(i==256+0x3c&&key_shift!=0){/*Shift+F2*/
                 /*新しく作ったコンソールを入力選択状態にする*/
                 if(key_win!=0){
                  keywin_off(key_win);
                 }
                 key_win=open_console(shtctl,memtotal);
                 sheet_slide(key_win,32,4);
                 sheet_updown(key_win,shtctl->top);
                 keywin_on(key_win);
				}
               if(i==256+0x57){
                 /*F11キーを押した場合*/
                 sheet_updown(shtctl->sheets[1],shtctl->top-1);
               }

               if(i==256+0xfa){/*キーボードがデータを無事に受け取った*/
                keycmd_wait=-1;
               }
               if(i==256+0xfe){/*キーボードがデータを無事に受け取れなかった*/
                wait_KBC_sendready();
                _io_out8(PORT_KEYDAT,keycmd_wait);
               }
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
               new_mx=mx;
               new_my=my;               
               if((mdec.btn&0x01)!=0){
                 /*左ボタンを押していたら、上の下敷きから順番にマウスが指している下敷きを探す*/
                 if(mmx<0){
                  /*通常モード*/
                   for(j=shtctl->top-1;j>0;j--){
                     sht=shtctl->sheets[j];
                     x=mx-sht->vx0;
                     y=my-sht->vy0;
                     if(0<=x&&x<sht->bxsize&&0<=y&&y<sht->bysize){
                       if(sht->buf[y*sht->bxsize+x]!=sht->col_inv){
                         sheet_updown(sht,shtctl->top-1);//ここで、下にあるウィンドウをクリックした場合に上に持ってくるようにしている
                         if(sht!=key_win){
                          keywin_off(key_win);
                          key_win=sht;
                          keywin_on(key_win);
                         }
                         if(3<=x&&x<sht->bxsize-3&&3<=y&&y<21){                          
                           /*これは、ウィンドウのタイトルの部分をクリックしている状態なので、ここから動かした分だけをウィンドウモードで移動させる*/
                           mmx=mx;/*ウィンドウモードへ移動*/

                           mmy=my;
                           mmx2=sht->vx0;
                           new_wy=sht->vy0;
                         }
                         if(sht->bxsize-21<=x&&x<sht->bxsize-5&&5<=y&&y<19){
                           /*☓ボタンクリックしている*/
                           if((sht->flags&0x10)!=0){
                            /*アプリが作ったウィンドウかどうかを判断*/
                             task=sht->task;                         
                             cons_putstr0(task->cons,"\nBreak(mouse)\n");
                            _io_cli();/*ここで、強制終了中にタスクが変わると困るため*/
                            task->tss.eax=(int)&(task->tss.esp0);//ここで強制アプリを終了するための準備
                            task->tss.eip=(int)asm_end_app;//強制終了
                            _io_sti();
                           }
                         }
                         break;
                       }
                     }
                   }
                }else{
                  /*ウィンドウ移動モード*/
                  x=mx-mmx;
                  y=my-mmy;
                  new_wx=(mmx2+x+2)&~3;
                  new_wy=new_wy+y;
                  /*移動後の座標更新*/
                  mmy=my;
                }        
               }else{
                 /*左ボダンを押してない*/
                 mmx=-1;/*通常モード*/
                 if(new_wx!=0x7fffffff){
                  sheet_slide(sht,new_wx,new_wy);//一度確定
                  new_wx=0x7fffffff;
                 }
               }
             }
           }else if(768<=i&&i<=1023){
		close_console(shtctl->sheets0 + (i - 768));
			}
       }
    }
}

void keywin_off(struct SHEET *key_win){
  change_wtitle8(key_win,0);
  if((key_win->flags&0x20)!=0){
    /*ここで、アプリかウィンドウかを判断するためにSHEET構造のflagsを使っている。なお、コンソールのカーソルが必要かどうかを0x20のビットで確認している*/
    fifo32_put(&key_win->task->fifo,3);/*コンソールのカーソルOFF*/
    }
  return;
}

void keywin_on(struct SHEET *key_win){
  change_wtitle8(key_win,1);
  if((key_win->flags&0x20)!=0){
    fifo32_put(&key_win->task->fifo,2);/*コンソールのカーソルをONにするようにする*/
    }
  return ;
}


