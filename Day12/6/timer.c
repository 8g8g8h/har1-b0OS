#include"bootpack.h"
#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040
#define TIMER_FLAGS_ALLOC	1	/*確保した状態*/
#define TIMER_FLAGS_USING	2	/*タイマ作動中*/
struct TIMERCTL timerctl;

void init_pit(void){
	int i;
	_io_out8(PIT_CTRL,0x34);
	_io_out8(PIT_CNT0,0x9c);
	_io_out8(PIT_CNT0,0x2e);
	timerctl.count=0;
	timerctl.next=0xffffffff;
	for(i=0;i<MAX_TIMER;i++){
		timerctl.timer[i].flags=0; /*未使用*/
	}
	return ;
}

//ここで使うタイマを設定している
//この関数は該当するタイマのアドレスを返すもの
//それが構造体なのでstruct型である
struct TIMER *timer_alloc(void){
	int i;
	for(i=0;i<MAX_TIMER;i++){
		if(timerctl.timer[i].flags==0){
			timerctl.timer[i].flags=TIMER_FLAGS_ALLOC;
			return &timerctl.timer[i];
		}
	}
	return 0;
}
//ここではタイマを解放する
void timer_free(struct TIMER *timer){
	timer->flags=0;
	return ;
}
//ここで該当するタイマの設定を行う
//タイマの構造体で設定されていないものがfifo,dataの2つである
void timer_init(struct TIMER *timer,struct FIFO8 *fifo,unsigned char data){
	timer->fifo=fifo;
	timer->data=data;
	return ;
}

void timer_settime(struct TIMER *timer,unsigned int timeout){
	timer->timeout=timeout+timerctl.count;//ここの意味は何秒後に処理をするかの設定である
	timer->flags=TIMER_FLAGS_USING;
	if(timerctl.next>timer->timeout){
		//次回のタイマの時刻を更新
		timerctl.next=timer->timeout;
	}
}

void inthandler20(int *esp){
	int i;
	_io_out8(PIC0_OCW2,0x60);/*PIC0にIRQ-00受付完了を通知*/
	timerctl.count++;
	if(timerctl.next>timerctl.count){
		return ;/*まだ現段会のタイマが切れていない(つまり次のタイマになっていない)のでここで終わる*/
	}
	timerctl.next=0xffffffff;
	for(i=0;i<MAX_TIMER;i++){
		if(timerctl.timer[i].flags==TIMER_FLAGS_USING){
			if(timerctl.timer[i].timeout<=timerctl.count){
				//ここでタイマが切れたことになる(つまり該当タイマが予定時刻になった)
				timerctl.timer[i].flags=TIMER_FLAGS_ALLOC;
				fifo8_put(timerctl.timer[i].fifo,timerctl.timer[i].data);
			}else{
				/*まだ予定時刻でない(つまりタイムアウトではない)*/
				if(timerctl.next>timerctl.timer[i].timeout){
					timerctl.next=timerctl.timer[i].timeout;
				}
			}
		}
	}
	return ;
}

