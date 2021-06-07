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
	timerctl.using=0;
	for(i=0;i<MAX_TIMER;i++){
		timerctl.timers0[i].flags=0; /*未使用*/
	}
	return ;
}

//ここで使うタイマを設定している
//この関数は該当するタイマのアドレスを返すもの
//それが構造体なのでstruct型である
struct TIMER *timer_alloc(void){
	int i;
	for(i=0;i<MAX_TIMER;i++){
		if(timerctl.timers0[i].flags==0){
			timerctl.timers0[i].flags=TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
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
void timer_init(struct TIMER *timer,struct FIFO32 *fifo,int data){
	timer->fifo=fifo;
	timer->data=data;
	return ;
}
//timersへの登録をするために一時的に割り込みを禁止する
void timer_settime(struct TIMER *timer,unsigned int timeout){
	int e,j,i;
	timer->timeout=timeout+timerctl.count;//ここの意味は何秒後に処理をするかの設定である
	timer->flags=TIMER_FLAGS_USING;
	e=_io_load_eflags();//今の割り込みフラグをeに格納する
	_io_cli();//割り込みフラグを1にして、割り込みを禁止する
	//どこに登録するかを調べる(ここでbreakした場所(i)に登録するを調べる)
	//例えばusingが5ならばtimersの配列の数は5だけれど要素としては4まで
	for(i=0;i<timerctl.using;i++){
		if(timerctl.timers[i]->timeout>=timer->timeout){
			break;
		}
	}
	//後ろをずらす
	//このあとに配列の数を増やすので[5]にはもともとデータが入っていない
	for(j=timerctl.using;j>i;j--){
		timerctl.timers[j]=timerctl.timers[j-1];
	}
	timerctl.using++;
	/*空いた隙間に入れる*/
	timerctl.timers[i]=timer;
	//次回のタイマの時刻を更新
	timerctl.next=timerctl.timers[0]->timeout;
	_io_store_eflags(e);//ここで割り込みをもとに戻してあげている
	return ;
	
}

void inthandler20(int *esp){
	int i,j;
	_io_out8(PIC0_OCW2,0x60);/*PIC0にIRQ-00受付完了を通知*/
	timerctl.count++;
	if(timerctl.next>timerctl.count){
		return ;/*まだ現段回のタイマが切れていない(つまり次のタイマになっていない)のでここで終わる*/
	}
	for(i=0;i<timerctl.using;i++){
		/*ここではすべてのタイマが動作しているためflagsを確認しない*/
		//↓のif文はcountがtimeoutに追いついていない場合を示しており、この場合まだタイマ[i]は時間になっていないのでその前まではタイムアウトになっている
		if(timerctl.timers[i]->timeout > timerctl.count){
			break;
		}
		/*タイムアウト*/
		timerctl.timers[i]->flags=TIMER_FLAGS_ALLOC;
		fifo32_put(timerctl.timers[i]->fifo,timerctl.timers[i]->data);
	}
	/*ちょうどi個のタイマがタイムアウトした。残りのタイマをずらす*/
	timerctl.using-=i;
	for(j=0;j<timerctl.using;j++){
		timerctl.timers[j]=timerctl.timers[i+j];
	}
	if(timerctl.using>0){
		//ずらしたため最初のタイマが一番短いものになる
		timerctl.next=timerctl.timers[0]->timeout;
	}else{
		timerctl.next=0xffffffff;
	}
	return ;
}

