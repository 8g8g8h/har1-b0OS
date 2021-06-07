/*naskfunc.asmで作った関数*/

/*io_hltを使うために最初に宣言する*/
extern void io_hlt(void);
extern void write_mem8(int addr,int data);

void HariMain(void){
  int i; /*変数宣言*/
  //0xa0000はVRAMの設定をしてあるメモリ番地。0xaffff(画面の下まで)まで書くのはなぜかというとINT　0x10に書かれている内容、ここのアドレスにデータを入れる
  for(i=0xa0000;i<=0xaffff;i++){
    write_mem8(i,6); /*MOV BYTE[i],15番が設定で白色*/
  }
  for(;;){
    io_hlt();
  }
}
