#include"bootpack.h"

struct TIMER *mt_timer;

int mt_tr;
//マルチタスクを作るために、まずタイマを確保しておく。
//そして時間をセットしておく(0.02秒後に設定しておく)
//ここでは、データを書き込まないのでtimer_initがいらない
//そして、いま実行しているタスクをmt_trに入れておく。

void mt_init(void){
  mt_timer=timer_alloc();
  /*timer_initはいらない*/
  timer_settime(mt_timer,2);
  mt_tr=3*8;
  return ;
}

void mt_taskswitch(void){
  if(mt_tr==3*8){
    mt_tr=4*8;
  }else{
    mt_tr=3*8;
  }
  timer_settime(mt_timer,2);
  farjmp(0,mt_tr);
  return ;
}
