#include"bootpack.h"
#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040
#define TIMER_FLAGS_ALLOC	1	/*確保した状態*/
#define TIMER_FLAGS_USING	2	/*タイマ作動中*/
struct TIMERCTL timerctl;

void init_pit(void){
	int i;
    struct TIMER *t;
	_io_out8(PIT_CTRL,0x34);
	_io_out8(PIT_CNT0,0x9c);
	_io_out8(PIT_CNT0,0x2e);
	timerctl.count=0;
	for(i=0;i<MAX_TIMER;i++){
		timerctl.timers0[i].flags=0; /*未使用*/
	}
    t=timer_alloc();/*1つもらってくる*/
    t->timeout=0xffffffff;//t(番兵)のtimeoutを設定
    t->flags=TIMER_FLAGS_USING;
    t->next=0;//一番うしろ
    timerctl.t0=t;//今は番兵しかいないので先頭でもあるためここ
	timerctl.next=0xffffffff;//番兵しかいないので番兵の時刻を入れる
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
        //sとtの間に入れる
        if(timer->timeout<=t->timeout){
            s->next=timer;//sの次はtimer
            timer->next=t;//timerの次はt
            _io_store_eflags(e);
            return ;
        }
    }
}

void inthandler20(int *esp){
  char ts=0;
  struct TIMER *timer;
	_io_out8(PIC0_OCW2,0x60);/*PIC0にIRQ-00受付完了を通知*/
	timerctl.count++;
	if(timerctl.next>timerctl.count){
		return ;/*まだ現段回のタイマが切れていない(つまり次のタイマになっていない)のでここで終わる*/
	}
    timer=timerctl.t0;// 先頭のタイマを代入
	for(;;){
		/*ここではすべてのタイマが動作しているためflagsを確認しない*/
		//↓のif文はcountがtimeoutに追いついていない場合を示しており、この場合まだタイマは時間になっていないのでその前まではタイムアウトになっている
        //最後に追いついたらbreakしており、そこでタイマが止まっている
		if(timer->timeout > timerctl.count){
			break;
		}
		/*タイムアウト*/
    timer->flags=TIMER_FLAGS_ALLOC;
    if(timer!=task_timer){
      fifo32_put(timer->fifo,timer->data);
    }else{
      ts=1;/*mt_timerがタイムアウトしたことを示している。*/
    }
    /*次のタイマの番地をtimerに入れる*/
    timer=timer->next;
  }
	/*ずらし
   *ここでタイムアウトしていない最初のものを[0]に代入している
   */
  timerctl.t0=timer;

  /*timerctl.nextの設定*/
  timerctl.next=timerctl.t0->timeout;
  if(ts!=0){
    task_switch();
  }
	return ;
}

//タイムアウトしたタイマがmt_timerであれば、fifoには書き込まずts=1にしている。そして最後にtsが0でなければ、mt_taskswitchを呼び出して、タスクスイッチを行う
