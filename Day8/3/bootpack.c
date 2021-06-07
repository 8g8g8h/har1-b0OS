#include<stdio.h>
#include"bootpack.h"



extern struct FIFO8 keyfifo,mousefifo;
void enable_mouse(struct MOUSE_DEC *mdec);
void init_keyboard(void);
int mouse_decode(struct MOUSE_DEC *mdec,unsigned char dat);

void HariMain(void){
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    char s[40];	/*デバッグに必要な変数*/
    char mcursor[256]; /*マウスを表記するための変数*//*マウスに必要な変数*/
    char keybuf[32];
    char mousebuf[128];
    int mx=(binfo->scrnx-16)/2; /*マウスのｘ座標*/
    int my=(binfo->scrny-28-16)/2; /*マウスのｙ座標*/
    int i;
    struct MOUSE_DEC mdec;

    init_gdtidt();
    init_pic();
    _io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
    

    fifo8_init(&keyfifo,32,keybuf);
    fifo8_init(&mousefifo,128,mousebuf);

    _io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) ここでは後ろから数値が入るつまり（10011111）の順番ではいる */
    _io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111)ここでも(11110111)で入るこれは0x2cに値する */

    init_keyboard();
    
    _init_palette();
    init_screen(binfo->vram,binfo->scrnx,binfo->scrny);
    

    init_mouse_cursor8(mcursor,COL8_008484);
    putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,mcursor,16);
    sprintf(s,"(%d,%d)",mx,my);
    putfont8_asc(binfo->vram,binfo->scrnx,0,0,COL8_FFFFFF,s);

    enable_mouse(&mdec);

    

    for(;;){
        _io_cli(); /*割り込み禁止*/
        if(fifo8_status(&keyfifo)+fifo8_status(&mousefifo)== 0){
            _io_stihlt(); /*割り込み禁止を解除しながらhlt(cpuを待機状態にする)*/
        } else {
            if(fifo8_status(&keyfifo)!=0){
                i=fifo8_get(&keyfifo);
                _io_sti();/*CPUの割り込み禁止解除*/

                sprintf(s, "%x",i);
                bookfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
                putfont8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            }else if(fifo8_status(&mousefifo)!=0){
                i=fifo8_get(&mousefifo);
                _io_sti();/*CPUの割り込み禁止解除*/
                if(mouse_decode(&mdec,i)!=0){
                    /*データが3バイト揃ったので表示させる*/
                    sprintf(s,"[lcr  %d  %d]",mdec.x,mdec.y);

                    if ((mdec.btn & 0x01) != 0) {
                      s[1] = 'L';
                    }
                    if ((mdec.btn & 0x02) != 0) {
                      s[3] = 'R';
                    }
                    if ((mdec.btn & 0x04) != 0) {
                      s[2] = 'C';
                    }
                    bookfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 64, 31);
                    putfont8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF,s);
                }
                     
            }

        }
            
    }
}
#define PORT_KEYDAT 0x0060 /*キーボードの制御回路のモードを設定する*/
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47   /*マウスの利用モードの番号*/

void wait_KBC_sendready(void){
    /* キーボードコントローラがデータ送信可能になるのを待つ */
    /*キーボード制御回路が制御命令を受け付けるまで待つ関数*/
    for(;;){
        if((_io_in8(PORT_KEYSTA)&KEYSTA_SEND_NOTREADY)==0){
            break;
        }
    }
    return;
}

void init_keyboard(void){
    /* キーボードコントローラの初期化 */
    wait_KBC_sendready();
    _io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);/*キーボード制御回路に送信できることを確認し、次に書き込んだデータをcpuに送信してくれる*/
    wait_KBC_sendready();
    _io_out8(PORT_KEYDAT,KBC_MODE); /*ここでモード設定する*/
    return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(struct MOUSE_DEC *mdec){
    /* マウス有効 */
    wait_KBC_sendready();
    _io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE); /*キーボード制御回路に確認、ここで次に書き込んだデータをマウスへ送信してくれる*/
    wait_KBC_sendready();
    _io_out8(PORT_KEYDAT,MOUSECMD_ENABLE); /*データを送る*/
     /* うまくいくとACK(0xfa)が送信されてくる */
     mdec->phase=0;/*マウスの0xfaを待っている段階*/
     return ;
} 

int mouse_decode(struct MOUSE_DEC *mdec,unsigned char dat){
    if(mdec->phase==0){
        /* マウスの0xfaを待っている段階へ */
        if(dat==0xfa){
            mdec->phase=1;
        }
		return 0;
    }
	if(mdec->phase==1){
        /* マウスの1バイト目を待っている段階へ */
        /*これは1バイト目が移動に反応する桁は0~3の範囲で、8~Fの範囲であるかを確認している*/
        if((dat&0xc8)==0x08){
            mdec->buf[0]=dat;
        	mdec->phase=2;
		}
        return 0;
    }
	if(mdec->phase==2){
        /* マウスの2バイト目を待っている段階へ */
        mdec->buf[1]=dat;
        mdec->phase=3;
        return 0;
    }
	if(mdec->phase==3){
        /* マウスの3バイト目を待っている段階へ */
        mdec->buf[2]=dat;
        mdec->phase=1;
        mdec->btn=mdec->buf[0]&0x07;/*ボタンの状態は下位3ビットに格納されているのでここで取り出している*/
        mdec->x=mdec->buf[1];
        mdec->y=mdec->buf[2];
		//ここでbuf[0]の1バイト目の移動に反応する桁の情報(0,1,2)によってきめている(下位1バイトより上を1か0にするか)
        if((mdec->buf[0]&0x10)!=0){
            mdec->x|=0xffffff00;
        }
        if((mdec->buf[0]&0x20)!=0){
            mdec->y|=0xffffff00;
        }
        mdec->y=-mdec->y;
        return 1;
    }
    return -1;
}
