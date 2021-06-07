#include<stdio.h>
#include"bootpack.h"

extern struct FIFO8 keyfifo;
void enable_mouse(void);
void init_keyboard(void);

void HariMain(void){
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    char s[40];	/*デバッグに必要な変数*/
    char mcursor[256]; /*マウスを表記するための変数*//*マウスに必要な変数*/
    char keybuf[32];
    int mx=(binfo->scrnx-16)/2; /*マウスのｘ座標*/
    int my=(binfo->scrny-28-16)/2; /*マウスのｙ座標*/
    int i;

    init_gdtidt();
    init_pic();
    _io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
    

    fifo8_init(&keyfifo,32,keybuf);

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

    

    for(;;){
        _io_cli(); /*割り込み禁止*/
        if(fifo8_status(&keyfifo)== 0){
            _io_stihlt(); /*割り込み禁止を解除しながらhlt(cpuを待機状態にする)*/
        } else {
            i=fifo8_get(&keyfifo);
            _io_sti();/*CPUの割り込み禁止解除*/

            sprintf(s, "%x",i);
			bookfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
			putfont8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
        }
    }
}

#define PORT_KEYDAT 0x0060 /*キーボードの制御回路のモードを設定する*/
#define PORT_KEYSTA 0x0064 /*キーボードコントローラーのステータス(状態)について(port番号)*/
#define PORT_KEYCMD 0x0064 /*キーボードコントローラーのステータス(状態)について(port番号)*/
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60 /*キーボードコントローラー*/
#define KBC_MODE 0x47   /*マウスの利用モードの番号*/

void wait_KBC_sendready(void){
    /* キーボードコントローラがデータ送信可能になるのを待つ */
	//キーボードコントローラーが命令を受け入れられるようになったら0x64番号の装置から読み取ったデータの下位2bit目が0になる
    /*キーボードコントローラーが制御命令を受け付けるまで待つ関数*/
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
    _io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);/*キーボードコントローラーに送信できることを確認し、モード設定(モードを使えるようにする)(0x60がモード設定のコマンド番号)を行っている(ステータスを変更している)*/
    wait_KBC_sendready();
    _io_out8(PORT_KEYDAT,KBC_MODE); /*ここでモード設定する(マウスが利用できるようにする,モード番号が0x47である)　キーボードエンコーダーにもコマンド番号を設定している*/
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(void){
    /* マウス有効 */
    wait_KBC_sendready();
    _io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE); /*キーボードコントローラーにマウスが有効化されているかを確認する(ここで次に書き込んだデータ(有効化するためのコマンド)をマウスへ送信)*/
    wait_KBC_sendready();
    _io_out8(PORT_KEYDAT,MOUSECMD_ENABLE); /*データを送る(ここでマウスを有効にしている)*/
    return; /* うまくいくとACK(0xfa)が送信されてくる */
} 
