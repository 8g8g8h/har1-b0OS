/*コンソール周りについて*/
#include<stdio.h>
#include"bootpack.h"

void console_task(struct SHEET *sheet,unsigned int memtotal){

  struct TIMER *timer;
  int i,fifobuf[128],cursor_x=16,cursor_y=28,cursor_c=-1,x,y;
  struct TASK *task=task_now();
  char s[30],cmdline[30],*p;
  struct MEMMAN *memman=(struct MEMMAN *)MEMMAN_ADDR;
  struct FILEINFO *finfo=(struct FILEINFO *)(ADR_DISKIMG+0x002600);
  int *fat=(int *)memman_alloc_4k(memman,4*2800);

  struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *)ADR_GDT;

  fifo32_init(&task->fifo,128,fifobuf,task);  
  timer=timer_alloc();
  timer_init(timer,&task->fifo,1);
  timer_settime(timer,50);

  file_readfat(fat,(unsigned char *)(ADR_DISKIMG+0x000200));/*ここでディスクイメージのFATの内容の圧縮を解く*/

  /*コマンドプロンプト表示*/
  putfonts8_asc_sht(sheet,8,28,COL8_FFFFFF,COL8_000000,">",1);

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
          if(cursor_c>=0){
            cursor_c=COL8_FFFFFF;
          }
        }else{
          timer_init(timer,&task->fifo,1);/*次は1*/
          if(cursor_c>=0){
            cursor_c=COL8_000000;
          }
        }
        timer_settime(timer,50);
      }
      if(i==2){/*カーソルON*/
        cursor_c=COL8_FFFFFF;
      }
      if(i==3){/*カーソルOFF*/
        bookfill8(sheet->buf,sheet->bxsize,COL8_000000,cursor_x,cursor_y,cursor_x+7,cursor_y+15);
        cursor_c=-1;
      }
      if(256<=i&&i<=511){
        if(i==8+256){
          /*バックスペース*/
          if(cursor_x>16){
            /*カーソルをスペースで消してから、カーソルを1つ戻す*/
            putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000," ",1);
            cursor_x-=8;
          }
        }else if(i==256+10){
          
          /*Enter */
          /*先に改行を行ってその後にコマンドかどうかを確認している。*/
          putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,    COL8_000000, " ",1);
          cmdline[cursor_x/8-2]=0;/*入力された文字の最後を0にして、文字列の最後を判断している*/
          cursor_y=cons_newline(cursor_y,sheet);/*ここで改行したときに行われる動作を行っている*/

          /*コマンド実行*/
          if(ostrcmp(cmdline,"mem")==0){
            /*memコマンド*/
            sprintf(s,"total  %dMB",memtotal/(1024*1024));
            putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,s,30);
            cursor_y=cons_newline(cursor_y,sheet);
            sprintf(s,"free %dKB",memman_total(memman)/1024);
            putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,s,30);
            cursor_y=cons_newline(cursor_y,sheet);/*free を書いた後の改行*/
            cursor_y=cons_newline(cursor_y,sheet);/*次のコマンドを入力するための改行*/
          }else if(ostrcmp(cmdline,"clear")==0){
            /*clearコマンド*/
            for(y=28;y<128+28;y++){
              for(x=8;x<8+240;x++){
                sheet->buf[x+y*sheet->bxsize]=COL8_000000;
              }
            }
            sheet_refresh(sheet,8,28,8+240,28+128);
            cursor_y=28;
          }else if(ostrcmp(cmdline,"ls")==0){
            /*lsコマンド*/
            for(x=0;x<224;x++){
              if(finfo[x].name[0]==0x00){
                break;
              }
              if(finfo[x].name[0]!=0xe5){
                if((finfo[x].type&0x18)==0){
                  sprintf(s,"filename.ext  %d",finfo[x].size);
                  for(y=0;y<8;y++){
                    s[y]=finfo[x].name[y];
                  }
                  s[9]=finfo[x].ext[0];
                  s[10]=finfo[x].ext[1];
                  s[11]=finfo[x].ext[2];
                  putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,s,30);
                  cursor_y=cons_newline(cursor_y,sheet);
                }
              }
            }
            cursor_y=cons_newline(cursor_y,sheet);
          }else if(ostrncmp(cmdline,"cat ",4)==0){
            /*catコマンド*/
            /*ファイル名を準備する*/
            for(y=0;y<11;y++){
              s[y]=' ';
            }
            y=0;
            for(x=4;y<11&&cmdline[x]!=0;x++){
              /*ここでyが8になるのは、sは、catコマンドの後のファイル名を入れる変数であり、名前は8byteまでしか使えないため、拡張子まで行っていてファイル名が8より小さかったら、y=8にすることにより、つぎの値では、yが拡張子を示すことができる*/
              if(cmdline[x]=='.'&&y<=8){
                y=8;
              }else{
                s[y]=cmdline[x];
                if('a'<=s[y]&&s[y]<='z'){
                  /*小文字は大文字に直す*/
                  s[y]-=0x20;
                }
                y++;
              }
            }
            /*ファイルを探す*/
            for(x=0;x<224;){
              if(finfo[x].name[0]==0x00){
                break;
              }
              if((finfo[x].type&0x18)==0){
                for(y=0;y<11;y++){
                  if(finfo[x].name[y]!=s[y]){
                    goto type_next_file;
                  }
                }
                break;/*ファイルが見つかった*/
              }
type_next_file:
              x++;              
            }
            if(x<224&&finfo[x].name[0]!=0x00){
              /*ファイルが見つかった場合*/
              p=(char *)memman_alloc_4k(memman,finfo[x].size);/*ファイルの内容を一回コピーしておくためのメモリを確保しておく*/
              file_loadfile(finfo[x].clustno,finfo[x].size,p,fat,(char *)(ADR_DISKIMG+0x003e00));/*ここでデータを取り出している。ここから以下のコードでデータをコマンドラインに表示する処理を行っている。*/
              cursor_x=8;
              for(y=0;y<finfo[x].size;y++){
                /*一文字ずつ表示*/
                s[0]=p[y];
                s[1]=0;
                if(s[0]==0x09){
                  /*タブが押された形跡がある(ファイルの中で)*/
                  for(;;){
                    putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000," ",1);
                    cursor_x+=8;
                    if(cursor_x==8+240){
                      cursor_x=8;
                      cursor_y=cons_newline(cursor_y,sheet);
                    }
                    if(((cursor_x-8)&0x1f)==0){
                      break;/*32で割り切れたらbreak*/
                    }
                  }
                }else if(s[0]==0x0a){
                  /*ファイルの中に改行があった場合*/
                  cursor_x=8;
                  cursor_y=cons_newline(cursor_y,sheet);
                }else if(s[0]==0x0d){
                  /*復帰*/
                }else{
                  /*普通の文字*/
                  putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000,s,1);
                  cursor_x+=8;
                  if(cursor_x==8+240){
                    /*右端に来たので改行する*/
                    cursor_x=8;
                    cursor_y=cons_newline(cursor_y,sheet);
                  }
                }
              }
              memman_free_4k(memman,(int)p,finfo[x].size);/*すべてのデータを表示し終わったので、一旦別のところにコピーするために取った確保されたメモリを解放している*/
            }else{
              /*ファイルが見つからなかった場合*/
              putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,"File not found.",15);
              cursor_y=cons_newline(cursor_y,sheet);
            }
            cursor_y=cons_newline(cursor_y,sheet);
          }else if(ostrcmp(cmdline,"hlt")==0){
            /*hlt.hrbアプリケーションを起動する*/
            for(y=0;y<11;y++){
              s[y]=' ';
            }
            s[0]='H';
            s[1]='L';
            s[2]='T';
            s[8]='H';
            s[9]='R';
            s[10]='B';
            for(x=0;x<224;){
              if(finfo[x].name[0]==0x00){
                break;
              }
              if((finfo[x].type&0x18)==0){
                for(y=0;y<11;y++){
                  if(finfo[x].name[y]!=s[y]){
                    goto hlt_next_file;
                  }
                }
                break;
              }
hlt_next_file :
              x++;
            }
            if(x<224&&finfo[x].name[0]!=0x00){
              /*ファイルが見つかった*/
              p=(char *)memman_alloc_4k(memman,finfo[x].size);
              file_loadfile(finfo[x].clustno,finfo[x].size,p,fat,(char *)(ADR_DISKIMG+0x003e00));
              set_segmdesc(gdt+1003,finfo[x].size-1,(int)p,AR_CODE32_ER);
              farjmp(0,1003*8);/*gdtのセグメントに1~2番目はdsctbl.cで3~1002番目まではmtask.cが使っているためこの番号*/
              memman_free_4k(memman,(int)p,finfo[x].size);
            }else{
              putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,"File not found.",15);
              cursor_y=cons_newline(cursor_y,sheet);
            }
            cursor_y=cons_newline(cursor_y,sheet);
          }else if(cmdline[0]!=0){
            /*コマンドでもなく、空行でもない*/
            putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,"Bad command.",12);
            cursor_y=cons_newline(cursor_y,sheet);
            cursor_y=cons_newline(cursor_y,sheet);
          }
          /*プロンプト表示*/
          putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,">",1);
          cursor_x=16;
        }else{
          /*一般文字*/
          if(cursor_x<240){
            /*一文字表示してから、カーソルを1つ進める*/
            s[0]=i-256;
            s[1]=0;
            cmdline[cursor_x/8-2]=i-256;
            putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000,s,1);
            cursor_x+=8;
          }
        }
      }
      if(cursor_c>=0){
        bookfill8(sheet->buf,sheet->bxsize,cursor_c,cursor_x,cursor_y,cursor_x+7,cursor_y+15);
      }
      sheet_refresh(sheet,cursor_x,cursor_y,cursor_x+8,cursor_y+16);      
    }
  }
}

int cons_newline(int cursor_y,struct SHEET *sheet){
  int x,y;
  if(cursor_y<28+112){
    cursor_y+=16;/*次の行*/
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
  return cursor_y;
}
