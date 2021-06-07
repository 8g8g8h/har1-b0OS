/*naskfunc.asmで作った関数*/

/*io_hltを使うために最初に宣言する*/
extern void _io_hlt(void);
extern void _write_mem8(int addr,int data);

void HariMain(void){
int i; /*変数宣言*/
char *p;
  for (i=0xa0000;i<0xaffff;i++){
    p=(char *)i; /*メモリ番地の代入*/
    *p=i&0x0f;/*メモリ番地の内容（色）を変更している*/
  }
  for(;;){
    _io_hlt();
  }
}
