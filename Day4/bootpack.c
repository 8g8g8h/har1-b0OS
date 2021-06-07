void _io_hlt(void);
void _write_mem8(int addr,int data);

void HariMain(void){
	int i;

	for(i=0xa000;i<=0xaffff;i++){
		write_mem8(i,15);
	}

	for(;;){
		_io_hlt();
	}

}
