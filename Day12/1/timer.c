#include"bootpack.h"
#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

void init_pit(void){
	//タイマを使うためのPIT(IRQ0)という装置のための設定
	//ここでは割り込みの周期を100Hzにしている
	//これで1秒間に100回割り込んでくれる(この割り込み)
	_io_out8(PIT_CTRL,0x34);
	_io_out8(PIT_CNT0,0x9c);//ここで下位8bitを入れている(割り込み周期の数字が16進数で0x2e9cでありその下8bit)
	_io_out8(PIT_CNT0,0x2e);//ここで上位8bitを入れる
	return ;
}

void inthandler20(int *esp){
	_io_out8(PIC0_OCW2,0x60);
	return ;
}
