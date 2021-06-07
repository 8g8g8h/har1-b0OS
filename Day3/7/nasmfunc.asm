; nasmfunc.asm
bits 32
global io_hlt
global write_mem8

section .text
io_hlt:
	HLT
	RET

write_mem8:         ;void write_mem8(int addr,int data);ESP+4=addrのアドレス,ESP+8=dataのアドレス
	MOV ECX, [ESP+4]  ;[ESP+4]にaddrが入っている
	MOV AL, [ESP+8]   ;[ESP+8]にdataが入っている
	MOV [ECX], AL
	RET
