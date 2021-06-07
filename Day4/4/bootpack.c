/*naskfunc.asmで作った関数*/

/*io_hltを使うために最初に宣言する*/
extern void _io_hlt(void);
extern void _write_mem8(int addr,int data);

void HariMain(void){
int i; /*変数宣言*/
char *p; /*pという変数は、BYTE用の番地を示す*/
p=0xa0000;
for (i=0;i<0xffff;i++){
    *(p+i)=i&0x0f;
}
for(;;){
_io_hlt();
}
}
