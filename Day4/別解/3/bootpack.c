void io_hlt(void);

void write_mem8(int addr, int data);


void HariMain(void){
	int i; /*32bit整数*/
	char *p;/*char型のポインタ(BYTE用番地を示す)*/

    for(i=0xa0000; i<=0xffff;i++){
	p=(char* )i; /*メモリ番地を入れる*/

        *p=i&0x0f;/*ポイント型変数pの中にiのメモリ番地が入っている。そしてAND演算によりまた新しいメモリ番地を入れる*/
    }

    for(;;){
       io_hlt();
    }
}
