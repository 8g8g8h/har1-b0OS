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
	int e;
    struct TIMER *t,*s;
	timer->timeout=timeout+timerctl.count;//ここの意味は何秒後に処理をするかの設定である
	timer->flags=TIMER_FLAGS_USING;
	e=_io_load_eflags();//今の割り込みフラグをeに格納する
	_io_cli();//割り込みフラグを1にして、割り込みを禁止する
	timerctl.using++;
    if(timerctl.using==1){
        /*動作中のタイマはこれ一つの場合*/
        timerctl.t0=timer;
        timer->next=0;//次はない
        timerctl.next=timer->timeout;
        _io_store_eflags(e);
        return ;
    }
    t=timerctl.t0;
    if(timer->timeout<=t->timeout){
        /*先頭に入れる*/
        timerctl.t0=timer;
        timer->next=t;//次はt
        timerctl.next=timer->timeout;
        _io_store_eflags(e);
        return ;
    }
    //どこに入れるかを探す
    for(;;){
        s=t;
        t=t->next;
        if(t==0){
            break;//一番うしろになった
        }
        //sとtの間に入れる
        if(timer->timeout<=t->timeout){
            s->next=timer;//sの次はtimer
            timer->next=t;//timerの次はt
            _io_store_eflags(e);
            return ;
        }
    }
    //一番うしろに入れる場合
    s->next=timer;
    timer->next=0;
    _io_store_eflags(e);    
    return ;
	
}

void inthandler20(int *esp){
	int i;
    struct TIMER *timer;
	_io_out8(PIC0_OCW2,0x60);/*PIC0にIRQ-00受付完了を通知*/
	timerctl.count++;
	if(timerctl.next>timerctl.count){
		return ;/*まだ現段回のタイマが切れていない(つまり次のタイマになっていない)のでここで終わる*/
	}
    timer=timerctl.t0;// 先頭のタイマを代入
	for(i=0;i<timerctl.using;i++){
		/*ここではすべてのタイマが動作しているためflagsを確認しない*/
		//↓のif文はcountがtimeoutに追いついていない場合を示しており、この場合まだタイマは時間になっていないのでその前まではタイムアウトになっている
        //最後に追いついたらbreakしており、そこでタイマが止まっている
		if(timer->timeout > timerctl.count){
			break;
		}
		/*タイムアウト*/
		timer->flags=TIMER_FLAGS_ALLOC;
		fifo32_put(timer->fifo,timer->data);
        timer=timer->next;
	}
	/*ちょうどi個のタイマがタイムアウトした。残りのタイマをずらす*/
	timerctl.using-=i;
	/*ずらし
     *ここでタイムアウトしていない最初のものを[0]に代入している
     * */

    timerctl.t0=timer;

	if(timerctl.using>0){
		//ずらしたため最初のタイマが一番短いものになる
		timerctl.next=timerctl.t0->timeout;
	}else{
		timerctl.next=0xffffffff;
	}
	return ;
}

