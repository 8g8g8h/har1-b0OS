#include<stdio.h>
#include"bootpack.h"

extern struct FIFO8 keyfifo,mousefifo;
void enable_mouse(void);
void init_keyboard(void);

void HariMain(void){
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    char s[40];	/*デバッグに必要な変数*/
    char mcursor[256]; /*マウスを表記するための変数*//*マウスに必要な変数*/
    char keybuf[32];
    char mousebuf[128];
    int mx=(binfo->scrnx-16)/2; /*マウスのｘ座標*/
    int my=(binfo->scrny-28-16)/2; /*マウスのｙ座標*/
    int i;
    unsigned char mouse_dbuf[3], mouse_phase;

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

    enable_mouse();
    mouse_phase=0; /* マウスの0xfaを待っている段階へ */

    

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

                if(mouse_phase==0){
                    /* マウスの0xfaを待っている段階へ */
                    if(i==0xfa){
                        mouse_phase=1;
                    }
                }else if(mouse_phase==1){
                        /* マウスの1バイト目を待っている段階へ */
                        mouse_dbuf[0]=i;
                        mouse_phase=2;
                }else if(mouse_phase==2){
                        /* マウスの2バイト目を待っている段階へ */
                        mouse_dbuf[1]=i;
                        mouse_phase=3;
                }else if(mouse_phase==3){
                        /* マウスの3バイト目を待っている段階へ */
                        mouse_dbuf[2]=i;
                        mouse_phase=1;
                        sprintf(s,"%x",mouse_dbuf[0]);
                        bookfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 47, 31);
                        putfont8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF,s);
                        sprintf(s,"%x",mouse_dbuf[1]);
                        bookfill8(binfo->vram, binfo->scrnx, COL8_008484, 64, 16, 79, 31);
                        putfont8_asc(binfo->vram, binfo->scrnx, 64, 16, COL8_FFFFFF,s);
                        sprintf(s,"%x",mouse_dbuf[2]);
                        bookfill8(binfo->vram, binfo->scrnx, COL8_008484, 96, 16, 111, 31);
                        putfont8_asc(binfo->vram, binfo->scrnx, 96, 16, COL8_FFFFFF,s);
                        
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
    _io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);/*キーボード制御回路に送信できることを確認し、モード設定を行っている*/
    wait_KBC_sendready();
    _io_out8(PORT_KEYDAT,KBC_MODE); /*ここでモード設定する*/
    return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(void){
    /* マウス有効 */
    wait_KBC_sendready();
    _io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE); /*キーボード制御回路に確認*/
    wait_KBC_sendready();
    _io_out8(PORT_KEYDAT,MOUSECMD_ENABLE); /*データを送る*/
    return; /* うまくいくとACK(0xfa)が送信されてくる */
} 
