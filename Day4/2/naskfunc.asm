;naskfunc.asm
;TAB=4

;オブジェクトファイルのための情報
section .text
	GLOBAL	_io_hlt	; このプログラムに含まれる関数名
	GLOBAL	_write_mem8;

;以下は実際の関数

;オブジェクトファイルではこれを書いてからプログラムを書く

_io_hlt:	;void io_hlt(void);
	HLT
	RET

_write_mem8:	;void write_mem(int addr,int data)
	MOV	ECX,[ESP+4]	;addrは[esp+4]に入る
	MOV	AL,[ESP+8]	;dataは[esp+8]に入る
	MOV	[ECX],AL
	RET
