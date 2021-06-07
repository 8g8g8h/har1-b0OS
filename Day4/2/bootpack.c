/*naskfunc.asmで作った関数*/

/*io_hltを使うために最初に宣言する*/
extern void _io_hlt(void);
extern void _write_mem8(int addr,int data);

void HariMain(void){
int i; /*変数宣言*/
for(i=0xa0000;i<=0xaffff;i++){
_write_mem8(i,i&0x0f); /*MOV BYTE[i],15*/
}
for(;;){
_io_hlt();
}

}
