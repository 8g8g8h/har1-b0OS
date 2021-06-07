/* マウス関係 */

#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;

void inthandler2c(int *esp)
    /*PS/2マウスからの割り込み*/
{
    int data;
    _io_out8(PIC1_OCW2,0x64); /*IRQ-12受付完了をPIC1に通知*/
    _io_out8(PIC0_OCW2,0x62); /*IRQ-02受付完了をPIC0に通知*/
    data = _io_in8(PORT_KEYDAT);
    fifo32_put(mousefifo,data+mousedata0);
    return ;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(struct FIFO32 *fifo,int data0,struct MOUSE_DEC *mdec){
    /*書き込み先のFIFOバッファを記憶*/
    mousefifo=fifo;
    mousedata0=data0;
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
        if((mdec->buf[0]&0x10)!=0){
            mdec->x|=0xffffff00;
        }
        if((mdec->buf[0]&0x20)!=0){
            mdec->y|=0xffffff00;
        }
        mdec->y= -mdec->y;
        return 1;
    }
    return -1;
}
