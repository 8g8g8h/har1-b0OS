#include"bootpack.h"
#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040
struct TIMERCTL timerctl;

void init_pit(void){
	_io_out8(PIT_CTRL,0x34);
	_io_out8(PIT_CNT0,0x9c);
	_io_out8(PIT_CNT0,0x2e);
	timerctl.count=0;
	return ;
}

void inthandler20(int *esp){
	_io_out8(PIC0_OCW2,0x60);
	timerctl.count++;
	return ;
}
