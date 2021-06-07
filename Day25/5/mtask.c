#include"bootpack.h"

struct TIMER *mt_timer;
struct TIMER *task_timer;
struct TASKCTL *taskctl;
//マルチタスクを作るために、まずタイマを確保しておく。
//そして時間をセットしておく(0.02秒後に設定しておく)
//ここでは、データを書き込まないのでtimer_initがいらない
//そして、いま実行しているタスクをmt_trに入れておく。

//これは、現在動作しているレベルで動作しているタスクの番地を教えてくれる関数
struct TASK *task_now(void){
  struct TASKLEVEL *tl=&taskctl->level[taskctl->now_lv];  /*ここで現在の一番上で動いているレベルを教えている*/

  return tl->tasks[tl->now]; /*ここでそのレベルのタスクで動いているものを教えている*/
}

void task_add(struct TASK *task){
  struct TASKLEVEL *tl=&taskctl->level[task->level];/*該当しているタスクのレベルを格納している*/
  tl->tasks[tl->running]=task;/*ここでタスクを追加している*/
  tl->running++;
  task->flags=2;/*動作中*/
  return ;
}

void task_remove(struct TASK *task){
  int i;
  struct TASKLEVEL *tl=&taskctl->level[task->level];

  /*該当しているtaskがどこにあるかを探す*/
  for(i=0;i<tl->running;i++){
    if(tl->tasks[i]==task){
      break;
    }
  }

  tl->running--;
  if(i<tl->now){
    tl->now--;/*ずれるのでこれも動かしておく*/
  }
  if(tl->now>=tl->running){
    tl->now=0;
  }

  task->flags=1;/*スリープに設定しておく*/

  /*ずらしをする*/
  for(;i<tl->running;i++){
    tl->tasks[i]=tl->tasks[i+1];
  }
  return ;
}

void task_switchsub(void){
  int i;
  /*一番上のレベルを探す*/
  for(i=0;i<MAX_TASKLEVELS;i++){
    if(taskctl->level[i].running>0){
      break;//見つかった
    }
  }
  taskctl->now_lv=i;
  taskctl->lv_change=0;/*ここでは、レベルを変える必要なし*/
  return ;
}

void task_idle(void){
  for(;;){
    _io_hlt();
  }
}


struct TASK *task_init(struct MEMMAN *memman){
  int i;
  struct TASK *task,*idle;
  struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *)ADR_GDT;
  taskctl=(struct TASKCTL *)memman_alloc_4k(memman,sizeof(struct TASKCTL));
  for(i=0;i<MAX_TASKS;i++){
    taskctl->tasks0[i].flags=0;
    taskctl->tasks0[i].sel=(TASK_GDT0+i)*8;
    set_segmdesc(gdt+TASK_GDT0+i,103,(int)&taskctl->tasks0[i].tss,AR_TSS32);
  }

  for(i=0;i<MAX_TASKLEVELS;i++){
    taskctl->level[i].running=0;
    taskctl->level[i].now=0;
  }

  task=task_alloc();
  task->flags=2;/*動作中マーク*/
  task->priority=2;/*0.02秒後にタスクスイッチするように優先度を設定している*/
  task->level=0;/*最高レベル*/
  task_add(task);
  task_switchsub();//レベル設定
  load_tr(task->sel);
  task_timer=timer_alloc();
  timer_settime(task_timer,task->priority);

  idle=task_alloc();
  idle->tss.esp=memman_alloc_4k(memman,64*1024)+64*1024;
  idle->tss.eip=(int)&task_idle;
  idle->tss.es=1*8;
  idle->tss.cs=2*8;
  idle->tss.ss=1*8;
  idle->tss.ds=1*8;
  idle->tss.fs=1*8;
  idle->tss.gs=1*8;
  task_run(idle,MAX_TASKLEVELS-1,1);


  return task;
}

struct TASK *task_alloc(void){
  int i;
  struct TASK *task;
  for(i=0;i<MAX_TASKS;i++){
    if(taskctl->tasks0[i].flags==0){
      task=&taskctl->tasks0[i];
      task->flags=1;/*使用中マーク*/
      task->tss.eflags=0x00000202;/*IF=1*/
      task->tss.eax=0;/*とりあえず0にしておく*/
      task->tss.ecx=0;
      task->tss.edx=0;
      task->tss.ebx=0;
      task->tss.ebp=0;
      task->tss.esi=0;
      task->tss.edi=0;
      task->tss.es=0;
      task->tss.ds=0;
      task->tss.fs=0;
      task->tss.gs=0;
      task->tss.ldtr=0;
      task->tss.iomap=0x40000000;
      task->tss.ss0=0;
      return task;
    }
  }
  return 0;/*もう全部使用中*/
}

void task_run(struct TASK *task,int level,int priority){
  if(level<0){
    level=task->level;//レベル変更をしない
  }
  //priority=0の場合は、優先度を変更しないという意味
  //これは、スリープから起こすときに使う
  if(priority>0){
    task->priority=priority;
  }
  if(task->flags==2&&task->level!=level){//動作レベルの変更
    task_remove(task);//ここでflagsは1になるので下のif文も実行される
  }
  if(task->flags!=2){
    /*スリープから起こされる場合*/
    task->level=level;
    task_add(task);
  }
  taskctl->lv_change=1;/*次回タスクスイッチのときに次に実行するレベルを見直す必要がある(なぜなら、レベルを変えたときにそのレベルのタスクがなかった場合などは別のレベルにする必要があるためにここで必ずチェックする必要がある)*/
  return ;
}

void task_switch(void){
  struct TASKLEVEL *tl=&taskctl->level[taskctl->now_lv];
  struct TASK *new_task,*now_task=tl->tasks[tl->now];
  tl->now++;
  if(tl->now==tl->running){
    tl->now=0;
  }
  //レベルの変更を行う
  if(taskctl->lv_change!=0){
    task_switchsub();
    tl=&taskctl->level[taskctl->now_lv];
  }
  new_task=tl->tasks[tl->now];
  timer_settime(task_timer,new_task->priority);
  if(new_task!=now_task){
    farjmp(0,new_task->sel);
  }
  return ;
}

void task_sleep(struct TASK *task){
  struct TASK *now_task;
  //指定しているタスクが動いていたら
  //まずタスクスイッチをさせて、動いていない状態にする
  if(task->flags==2){
    //動作中だった場合
    now_task=task_now();
    task_remove(task);//これを実行するとflagsは1になる
    if(task==now_task){
      /*自分自身のスリープだったので、タスクスイッチが必要*/
      task_switchsub();
      now_task=task_now();//設定後での、「現在のタスク」を教えてもらう
      farjmp(0,now_task->sel);//スリープするのが自分だったら、すぐに次のタスクに移動
    }
  }
  return ;
}


