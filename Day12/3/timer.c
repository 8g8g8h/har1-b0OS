#include"bootpack.h"
#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040
struct TIMERCTL timerctl;

void init_pit(void){
	_io_out8(PIT_CTRL,0x34);
	_io_out8(PIT_CNT0,0x9c);
	_io_out8(PIT_CNT0,0x2e);
	timerctl.count=0;
	timerctl.timeout=0;
	return ;
}

void inthandler20(int *esp){
	//ここでIRQ番号0番(割り込みの種類の1種)が発生し、それを認識したことをPICに伝える
	_io_out8(PIC0_OCW2,0x60);
	timerctl.count++;
	if(timerctl.timeout>0){
		timerctl.timeout--;
		if(timerctl.timeout==0){
			fifo8_put(timerctl.fifo,timerctl.data);
		}
	}
	return ;
}
//ここれは設定が完全に終わる前に割り込みが入るのを防ぐため、設定したあとに割り込み状態にする
void settimer(unsigned int timeout,struct FIFO8 *fifo,unsigned char data){
	int eflags;
	eflags=_io_load_eflags();//ここで割り込みフラグを記録
	_io_cli();/*割り込み無効にする(フラグを0にすることで可能にする)*/
	//ここから下で設定を行っている
	timerctl.timeout=timeout;
	timerctl.fifo=fifo;//ここでfifo(データを運ぶための構造体)の設定を行っている
	timerctl.data=data;
	_io_store_eflags(eflags);//割り込みフラグをもとに戻す
	return ;
}
