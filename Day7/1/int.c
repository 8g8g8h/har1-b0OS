#include "bootpack.h"
#include<stdio.h>
/*PICの初期化*/
void init_pic(void){
    _io_out8(PIC0_IMR,  0xff  ); /* 全ての割り込みを受け付けない */
    _io_out8(PIC1_IMR,  0xff  ); /* 全ての割り込みを受け付けない */

    _io_out8(PIC0_ICW1,  0x11  ); /* エッジトリガモード */
    _io_out8(PIC0_ICW2,  0x20  ); /* IRQ0-7は、INT20-27で受ける*/
    _io_out8(PIC0_ICW3,  1<<2  ); /* PIC1はIRQ2にて接続 */
    _io_out8(PIC0_ICW4,  0x01  ); /* ノンバッファモード */

    _io_out8(PIC1_ICW1,  0x11  ); /* エッジトリガモード */
    _io_out8(PIC1_ICW2,  0x28  ); /* IRQ8-15は、INT28-2fで受ける */
    _io_out8(PIC1_ICW3,  2  ); /* PIC1はIRQ2似て接続 */
    _io_out8(PIC1_ICW4,  0x01  ); /* ノンバッファモード */
    
    _io_out8(PIC0_IMR,  0xfb  ); /*11111011 PIC1以外は全て禁止 */
    _io_out8(PIC1_IMR,  0xff  ); /*1111111 全ての割り込みを受け付けない */

    return;
}

#define PORT_KEYDAT		0x0060

void inthandler21(int *esp)
    /*PS/2キーボードからの割り込み*/
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    unsigned char data, s[4];
    _io_out8(PIC0_OCW2, 0x61);	/* CPUからIRQ-01受付完了をPICに通知を出力(ここで割り込みを元の状態に戻す　これによりまた割り込みをしても反応ができる) */
    data = _io_in8(PORT_KEYDAT);/*キーボードのキーが押された場合スキャンコードをキーボードコントローラーに格納する(スキャンコードはキーボードコントローラーに対して発行されるもの)ここで設定をしておくことでもう一回押しても反応されるようになる*/

	sprintf(s, "%x", data);
	bookfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
	putfont8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);

	return;
}

void inthandler2c(int *esp)
    /*PS/2マウスからの割り込み*/
{
    struct BOOTINFO *binfo=(struct BOOTINFO *) ADR_BOOTINFO;
    bookfill8(binfo->vram,binfo->scrnx,COL8_00FFFF,0,0,32*8-1,15);
    putfont8_asc(binfo->vram,binfo->scrnx,0,0,COL8_FFFFFF,"INT 2C (IRQ-12) : PS/2 mouse");
    for(;;){
        _io_hlt();
    }
}
void inthandler27(int *esp)
    /* PIC0からの不完全割り込み対策 */
/* Athlon64X2機などではチップセットの都合によりPICの初期化時にこの割り込みが1度だけおこる */
/* この割り込み処理関数は、その割り込みに対して何もしないでやり過ごす */
/* なぜ何もしなくていいの？
	→  この割り込みはPIC初期化時の電気的なノイズによって発生したものなので、
		まじめに何か処理してやる必要がない。									*/
{
	_io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知(7-1参照) */
	return;
}
