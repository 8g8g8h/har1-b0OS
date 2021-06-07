#include"bootpack.h"

#define PORT_KEYSTA 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47   /*マウスの利用モードの番号*/

struct FIFO32 *keyfifo;
int keydata0;

void inthandler21(int *esp)
    /*PS/2キーボードからの割り込み*/
{
    int data;
    _io_out8(PIC0_OCW2,0x61); /*IRQ-01受付完了をPICに通知*/
    data = _io_in8(PORT_KEYDAT);
    fifo32_put(keyfifo,data+keydata0);
    return ;
}


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

void init_keyboard(struct FIFO32 *fifo,int data0){
    /*書き込み先のFIFOバッファを記憶*/
    //data0に自分が決めた(READ.txtに書いてあるルール(キーボードなので256))を入れる
    keyfifo=fifo;
    keydata0=data0;
    /* キーボードコントローラの初期化 */
    wait_KBC_sendready();
    _io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);/*キーボード制御回路に送信できることを確認し、次に書き込んだデータをcpuに送信してくれる*/
    wait_KBC_sendready();
    _io_out8(PORT_KEYDAT,KBC_MODE); /*ここでモード設定する*/
    return;
}
