/*naskfunc.asmで作った関数*/

/*io_hltを使うために最初に宣言する*/
extern void _io_hlt(void);

void HariMain(void){
fin:
_io_hlt();
goto fin;

}
