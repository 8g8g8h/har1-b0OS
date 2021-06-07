/*コンソール周りについて*/
#include<stdio.h>
#include"bootpack.h"

void console_task(struct SHEET *sheet,unsigned int memtotal){

  struct TIMER *timer;
  struct TASK *task=task_now();
  struct MEMMAN *memman=(struct MEMMAN *)MEMMAN_ADDR;
  int i,fifobuf[128],*fat=(int *)memman_alloc_4k(memman,4*2880);
  struct CONSOLE cons;
  char cmdline[30];
  cons.sht=sheet;
  cons.cur_x=8;
  cons.cur_y=28;
  cons.cur_c=-1;
  *((int*)0x0fec)=(int)&cons; /*ここでconsのアドレスを別のところに書くようにしている*/ 

  fifo32_init(&task->fifo,128,fifobuf,task);  
  timer=timer_alloc();
  timer_init(timer,&task->fifo,1);
  timer_settime(timer,50);
  file_readfat(fat,(unsigned char *)(ADR_DISKIMG+0x000200));/*ここでディスクイメージのFATの内容の圧縮を解く*/

  /*コマンドプロンプト表示*/
  cons_putchar(&cons,'>',1);

  for(;;){
    _io_cli();
    if(fifo32_status(&task->fifo)==0){
      task_sleep(task);
      _io_sti();
    }else {
      i=fifo32_get(&task->fifo);
      _io_sti();
      if(i<=1){/*カーソル用タイマ*/
        if(i!=0){
          timer_init(timer,&task->fifo,0);/*次は0*/
          if(cons.cur_c>=0){
            cons.cur_c=COL8_FFFFFF;
          }
        }else{
          timer_init(timer,&task->fifo,1);/*次は1*/
          if(cons.cur_c>=0){
            cons.cur_c=COL8_000000;
          }
        }
        timer_settime(timer,50);
      }
      if(i==2){/*カーソルON*/
        cons.cur_c=COL8_FFFFFF;
      }
      if(i==3){/*カーソルOFF*/
        bookfill8(sheet->buf,sheet->bxsize,COL8_000000,cons.cur_x,cons.cur_y,cons.cur_x+7,cons.cur_y+15);
        cons.cur_c=-1;
      }
      if(256<=i&&i<=511){
        if(i==8+256){
          /*バックスペース*/
          if(cons.cur_x>16){
            /*カーソルをスペースで消してから、カーソルを1つ戻す*/
            cons_putchar(&cons,' ',0);
            cons.cur_x-=8;
          }
        }else if(i==256+10){
          
          /*Enter */
          /*先に改行を行ってその後にコマンドかどうかを確認している。*/
          cons_putchar(&cons,' ',0);
          cmdline[cons.cur_x/8-2]=0;/*入力された文字の最後を0にして、文字列の最後を判断している*/
          cons_newline(&cons);/*ここで改行したときに行われる動作を行っている*/
          cons_runcmd(cmdline,&cons,fat,memtotal);/*コマンド実行*/
          /*コマンドプロンプト表示*/
          cons_putchar(&cons,'>',1);
        }else{
          /*一般文字*/
          if(cons.cur_x<240){
            /*一文字表示してから、カーソルを1つ進める*/
            cmdline[cons.cur_x/8-2]=i-256;
            cons_putchar(&cons,i-256,1);
          }
        }
      }
      /*カーソル表示*/
      if(cons.cur_c>=0){
        bookfill8(sheet->buf,sheet->bxsize,cons.cur_c,cons.cur_x,cons.cur_y,cons.cur_x+7,cons.cur_y+15);
      }
      sheet_refresh(sheet,cons.cur_x,cons.cur_y,cons.cur_x+8,cons.cur_y+16);     
    }
  }
}

/*ファイルの文字列などを一個ずづ読んで描画する関数*/
void cons_putchar(struct CONSOLE *cons,int chr,char move){
  char s[2];
  s[0]=chr;
  s[1]=0;
  if(s[0]==0x09){
    /*tab*/
    for(;;){
      putfonts8_asc_sht(cons->sht,cons->cur_x,cons->cur_y,COL8_FFFFFF,COL8_000000," ",1);
      cons->cur_x+=8;
      if(cons->cur_x==8+240){
        cons_newline(cons);
      }
      if(((cons->cur_x-8)&0x1f)==0){
        break;/*32で割り切れたらbreak ここについては、調べて追記すること*/
      }
    }
  }else if(s[0]==0x0a){
    /*改行*/
    cons_newline(cons);
  }else if(s[0]==0x0d){
    /*復帰*/
    /*何もしない*/
  }else{
    /*普通の文字*/
    putfonts8_asc_sht(cons->sht,cons->cur_x,cons->cur_y,COL8_FFFFFF,COL8_000000,s,1);
    if(move!=0){
      /*moveが0のときはカーソルを進めない*/
      cons->cur_x+=8;
      if(cons->cur_x==8+240){
        cons_newline(cons);
      }
    }
  }
  return ;
}

/*改行やスクロールを描画するための関数*/
void cons_newline(struct CONSOLE *cons){
  int x,y;
  struct SHEET *sheet=cons->sht;
  if(cons->cur_y<28+112){
    cons->cur_y+=16;/*次の行*/
  }else{
    /*スクロール*/
    for(y=28;y<28+112;y++){
      for(x=8;x<8+240;x++){
        sheet->buf[x+y*sheet->bxsize]=sheet->buf[x+(y+16)*sheet->bxsize];
      }
    }
    for(y=28+112;y<28+128;y++){
      for(x=8;x<8+240;x++){
        sheet->buf[x+y*sheet->bxsize]=COL8_000000;
      }
    }
    sheet_refresh(sheet,8,28,8+240,28+128);
  }
  cons->cur_x=8;
  return ;
}

void cons_runcmd(char *cmdline,struct CONSOLE *cons,int *fat,unsigned int memtotal){
  if(ostrcmp(cmdline,"mem")==0){
    cmd_mem(cons,memtotal);
  }else if(ostrcmp(cmdline,"clear")==0){
    cmd_clear(cons);
  }else if(ostrcmp(cmdline,"ls")==0){
    cmd_ls(cons);
  }else if(ostrncmp(cmdline,"cat ",4)==0){
    cmd_cat(cons,fat,cmdline);
  }else if(cmdline[0]!=0){
    if(cmd_app(cons,fat,cmdline)==0){/*ここで一回cmd_appを実行している*/
      /*コマンドでもなく、アプリでもなく、空行でもない*/
      cons_putstr0(cons,"Bad command.\n\n");
    }
  }
  return ;
}

void cmd_mem(struct CONSOLE *cons,unsigned int memtotal){
  struct MEMMAN *memman=(struct MEMMAN *)MEMMAN_ADDR;
  char s[60];
  sprintf(s,"total  %dMB\n\nfree %dKB\n\n",memtotal/(1024*1024),memman_total(memman)/1024);
  cons_putstr0(cons,s);
  return ;
}

void cmd_clear(struct CONSOLE *cons){
  int x,y;
  struct SHEET *sheet=cons->sht;
  for(y=28;y<28+128;y++){
    for(x=8;x<8+240;x++){
      sheet->buf[x+y*sheet->bxsize]=COL8_000000;
    }
  }
  sheet_refresh(sheet,8,28,8+240,28+128);
  cons->cur_y=28;
  return ;
}

void cmd_ls(struct CONSOLE *cons){
  struct FILEINFO *finfo=(struct FILEINFO *)(ADR_DISKIMG+0x002600);
  int i,j;
  char s[30];
  for(i=0;i<224;i++){
    if(finfo[i].name[0]==0x00){
      break;
    }
    if(finfo[i].name[0]!=0xe5){
      if((finfo[i].type&0x18)==0){
        sprintf(s,"filename.ext  %d\n",finfo[i].size);
        for(j=0;j<8;j++){
          s[j]=finfo[i].name[j];
        }
        s[9]=finfo[i].ext[0];
        s[10]=finfo[i].ext[1];
        s[11]=finfo[i].ext[2];
        cons_putstr0(cons,s);
      }
    }
  }
  cons_newline(cons);
  return ;
}

void cmd_cat(struct CONSOLE *cons,int *fat,char *cmdline){
  struct MEMMAN *memman=(struct MEMMAN *)MEMMAN_ADDR;
  struct FILEINFO *finfo=file_search(cmdline+4,(struct FILEINFO *)(ADR_DISKIMG+0x002600),224);
  char *p;
  int i;
  if(finfo!=0){
    /*ファイルが見つかった場合*/
    p=(char *)memman_alloc_4k(memman,finfo->size);
    file_loadfile(finfo->clustno,finfo->size,p,fat,(char *)(ADR_DISKIMG+0x003e00)); 
    cons_putstr1(cons,p,finfo->size);
   
    memman_free_4k(memman,(int)p,finfo->size);
  }else {
    /*ファイルが見つからなかった場合*/
    cons_putstr0(cons,"File not found.\n");
  }
  cons_newline(cons);
  return ;
}

int cmd_app(struct CONSOLE *cons,int *fat,char *cmdline){
  struct MEMMAN *memman=(struct MEMMAN *)MEMMAN_ADDR;
  struct FILEINFO *finfo;
  struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *)ADR_GDT;
  struct TASK *task=task_now();
  char name[18];
  char *p;/*アプリのコードセグメント用のメモリ*/
  char *q;/*アプリのデータセグメン卜用のメモリ*/
  int i;
  int segsiz,datsiz,esp,dathrb;
  /*コマンドラインからファイル名を生成*/
  for(i=0;i<13;i++){
    if(cmdline[i]<=' '){
      break;
    }
    name[i]=cmdline[i];
  }
  name[i]=0;/*とりあえずファイル名の後ろを0にする*/

  /*ファイルを探す*/
  finfo=file_search(name,(struct FILEINFO *)(ADR_DISKIMG+0x002600),224);
  if(finfo==0&&name[i-1]!='.'){
    /*見つからなかったので、後ろに.hrbをつけてもう一度探してみる*/
    name[i ]='.';
    name[i+1]='H';
    name[i+2]='R';
    name[i+3]='B';
    name[i+4]=0;
    finfo=file_search(name,(struct FILEINFO *)(ADR_DISKIMG+0x002600),224);
  }

  if(finfo!=0){
    /*ファイルが見つかった場合*/    
    p=(char *)memman_alloc_4k(memman,finfo->size);
    file_loadfile(finfo->clustno,finfo->size,p,fat,(char *)(ADR_DISKIMG+0x003e00));
    if(finfo->size>=36&&ostrncmp(p+4,"Hari",4)==0&&*p==0x00){
      segsiz=*((int*)(p+0x0000));/*hrbファイルの構造の最初の部分(データセグメントの大きさ)*/
      esp=*((int *)(p+0x000c));/*ESPの初期値&データ部分のデータセグメントでの転送先番地*/
      datsiz=*((int *)(p+0x0010));/*hrbファイル内のデータ部分の大きさ*/
      dathrb=*((int *)(p+0x0014));/*hrbのファイルの中にあるデータ部分がどこから始まっているか*/
      q=(char *)memman_alloc_4k(memman,segsiz);
      *((int *)0xfe8)=(int)q;
      set_segmdesc(gdt+1003,finfo->size-1,(int)p,AR_CODE32_ER+0x60);/*アプリのコードセグメントを設定してあげている*/
      set_segmdesc(gdt+1004,segsiz-1,(int)q,AR_DATA32_RW+0x60);/*アプリのデータセグメントを設定してあげている*/
      for(i=0;i<datsiz;i++){
        q[esp+i]=p[dathrb+i];/*ここで、hrbファイル内のデータをデータセグメントにコピーしている*/
      }
      start_app(0x1b,1003*8,esp,1004*8,&(task->tss.esp0));
      memman_free_4k(memman,(int)q,segsiz);
    }else{
      cons_putstr0(cons,".hrb file format error.\n");
    }
    memman_free_4k(memman,(int)p,finfo->size);
    cons_newline(cons);
    return 1;
  }
  /*ファイルが見つからなかった場合*/
  return 0;
}

void cons_putstr0(struct CONSOLE *cons,char *s){
  for(;*s!=0;s++){
    cons_putchar(cons,*s,1);
  }
  return ;
}

void cons_putstr1(struct CONSOLE *cons,char *s,int l){
  int i;
  for(i=0;i<l;i++){
    cons_putchar(cons,s[i],1);
  }
  return ;
}

int *hrb_api(int edi,int esi,int ebp,int esp,int ebx,int edx,int ecx,int eax){
  struct CONSOLE *cons=(struct CONSOLE *)*((int *)0x0fec);
  int ds_base=*((int *)0xfe8);
  struct TASK *task=task_now();
  char s[12];
  struct SHTCTL *shtctl=(struct SHTCTL *)*((int*)0x0fe4);
  struct SHEET *sht;
  int *regs=&eax+1;/*ここで、最初に保存してある(asm_hrb_apiで最初に行ったPUSHAD)の内容をこれで示している。スタックは積まれていくごとにアドレスが引かれていくので、eaxの次のアドレスからは、最初のアドレスになる。*/
  if(edx==1){
    cons_putchar(cons,eax&0xff,1);
  }else if(edx==2){
    cons_putstr0(cons,(char *)ebx+ds_base);
  }else if(edx==3){
    cons_putstr1(cons,(char *)ebx+ds_base,ecx);
  }else if(edx==4){
    return &(task->tss.esp0);//ここでreturn する値がend_appのEAXになる
  }else if(edx==5){
    sht=sheet_alloc(shtctl);
    sheet_setbuf(sht,(char *)ebx+ds_base,esi,edi,eax);
    make_window8((char *)ebx+ds_base,esi,edi,(char *)ecx+ds_base,0);
    sheet_slide(sht,100,50);
    sheet_updown(sht,3);/*3という高さはtask_aの上*/
    regs[7]=(int)sht;
  }else if(edx==6){
    sht=(struct SHEET *)(ebx&0xfffffffe);
    putfont8_asc(sht->buf,sht->bxsize,esi,edi,eax,(char *)ebp+ds_base);
    if((ebx&1)==0){
      sheet_refresh(sht,esi,edi,esi+ecx*8,edi+16);
    }
  }else if(edx==7){
    sht=(struct SHEET *)(ebx&0xfffffffe);/*ここで、行っているのは、2の倍数になるように切り捨てている*/
    bookfill8(sht->buf,sht->bxsize,ebp,eax,ecx,esi,edi);
    if((ebx&1)==0){
      sheet_refresh(sht,eax,ecx,esi+1,edi+1);
    }
  }else if(edx==8){
    memman_init((struct MEMMAN *)(edx+ds_base)); /*memman_initでメモリについて初期化(ちなみに、structでmemman の構造にしないといけない)*/
    ecx&=0xfffffff0;/*16byte単位でスタートしている*/
    memman_free((struct MEMMAN *)(ebx+ds_base),eax,ecx);/*初期化している段階なので、最初に与えられているメモリを解放して、新たに設定できる用にしている。*/
  }else if(edx==9){
    ecx=(ecx+0x0f)&0xfffffff0;
    regs[7]=memman_alloc((struct MEMMAN *)(ebx+ds_base),ecx);/*メモリを確保している。その番地をregs[7]に代入する*/
  }else if(edx==10){
    ecx=(ecx+0x0f)&0xfffffff0;
    memman_free((struct MEMMAN *)(ebx+ds_base),eax,ecx);/*使っているメモリを確保する*/
  } else if(edx==11){
    sht=(struct SHEET *)(ebx&0xfffffffe);
    sht->buf[sht->bxsize*edi+esi]=eax;
    if((ebx&1)==0){
      sheet_refresh(sht,esi,edi,esi+1,edi+1);
    }
  }else if(edx==12){
    sht=(struct SHEET *)ebx;
    sheet_refresh(sht,eax,ecx,esi,edi);
  }
  return 0;
}

int *inthandler0d(int *esp){
  struct CONSOLE *cons=(struct CONSOLE *)*((int *)0x0fec);
  struct TASK *task=task_now();
  char s[30];
  cons_putstr0(cons,"\nINT 0D :\n General Protected Exception.\n" );
  sprintf(s,"EIP=%x\n",esp[11]);
  cons_putstr0(cons,s);
  return &(task->tss.esp0); /*異常終了*/
}

int *inthandler0c(int *esp){
  struct CONSOLE *cons=(struct CONSOLE *)*((int *)0x0fec);
  struct TASK *task=task_now();
  char s[30];
  cons_putstr0(cons,"\nINT 0C :\n Stack Exception.\n");
  sprintf(s,"EIP=%x\n",esp[11]);
  cons_putstr0(cons,s);
  return &(task->tss.esp0);
}
