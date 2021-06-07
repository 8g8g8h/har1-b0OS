#include "bootpack.h"
/*PICの初期化*/
void init_pic(void){
	//_io_out8(int port ,int value)
	//IMR=割り込み目隠しレジスタ(1になっているものを無視する)
    //ICW3は00000100を設定。ICW2はどの割り込みかを設定する
	//
	_io_out8(PIC0_IMR,  0xff  ); /* 全ての割り込みを受け付けない */
    _io_out8(PIC1_IMR,  0xff  ); /* 全ての割り込みを受け付けない */

    _io_out8(PIC0_ICW1,  0x11  ); /* エッジトリガモード */
    _io_out8(PIC0_ICW2,  0x20  ); /* IRQ0-7は、INT20-27で受ける*/
    _io_out8(PIC0_ICW3,  1<<2  ); /* PIC1(スレーブPIC)はIRQ2にて接続 */
    _io_out8(PIC0_ICW4,  0x01  ); /* ノンバッファモード */

    _io_out8(PIC1_ICW1,  0x11  ); /* エッジトリガモード */
    _io_out8(PIC1_ICW2,  0x28  ); /* IRQ8-15は、INT28-2fで受ける */
    _io_out8(PIC1_ICW3,  2  ); /* PIC1はIRQ2似て接続 */
    _io_out8(PIC1_ICW4,  0x01  ); /* ノンバッファモード */
    
    _io_out8(PIC0_IMR,  0xfb  ); /*11111011 PIC1以外は全て禁止 */
    _io_out8(PIC1_IMR,  0xff  ); /*1111111 全ての割り込みを受け付けない */

    return;
}
