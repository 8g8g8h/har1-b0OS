;naskfunc.asm
;TAB=4
; このプログラムに含まれる関数名
	GLOBAL	_io_hlt,_io_cli,_io_sti,_io_stihlt
	GLOBAL	_io_in8,_io_in16,_io_in32
	GLOBAL	_io_out8,_io_out16,_io_out32
	GLOBAL	_io_load_eflags,_io_store_eflags
	GLOBAL  load_gdtr,load_idtr
	GLOBAL	asm_inthandler21, asm_inthandler27, asm_inthandler2c
	EXTERN	inthandler21, inthandler27, inthandler2c
	
;オブジェクトファイルのための情報
section .text

	

;以下は実際の関数

;オブジェクトファイルではこれを書いてからプログラムを書く

_io_hlt:	;void io_hlt(void);
	HLT
	RET
_io_cli :	;void io_cli(void)(割り込みフラグを0にする);
	CLI
	RET

_io_sti	:	;void io_sti(void)(割り込みフラグを1にする);
	STI
	RET

_io_stihlt :	;void io_stihlt(void);
	STI
	HLT
	RET

_io_in8	:	;int io_in8(int port);

	MOV 	EDX ,[esp+4]	;port
	MOV	EAX,0
	IN	AL,DX
	RET

_io_in16	:	;int io_in16(int port);

	MOV 	EDX ,[esp+4]	;port
	MOV	EAX,0
	IN	AX,DX
	RET

_io_in32	:	;int io_in32(int port);

	MOV 	EDX ,[esp+4]	;port
	IN	EAX,DX
	RET

_io_out8	:	;int io_out8(int port,int data);これは指定されたportにデータを送る関数

	MOV 	EDX ,[esp+4]	;port
	MOV	AL,[esp+8]	;data
	OUT	DX,AL
	RET

_io_out16	:	;int io_out16(int port);

	MOV 	EDX ,[esp+4]	;port
	MOV	EAX,[esp+8]	;data
	OUT	DX,AX
	RET
_io_out32	:	;int io_out32(int port);

	MOV 	EDX ,[esp+4]	 ;port
	MOV	EAX,[esp+8]	;data
	OUT	DX,EAX		;32
	RET

_io_load_eflags	:	;int io_in8(int port);

	PUSHFD			;PUSH FLAGS という意味（double word）
	POP		EAX
	RET

_io_store_eflags	:	;int io_in8(int port);

	MOV 	EAX ,[ESP+4]
	PUSH	EAX
	POPFD			;POP FLAGSという意味
	RET

load_gdtr	:	;void load_gdtr(int limit,int addr);
	
	MOV	AX,[ESP+4]	;limit
	MOV	[ESP+6],AX
	LGDT	[ESP+6]
	RET

load_idtr	:	;void load_idtr(int limit,int addr);
	
	MOV	AX,[ESP+4]	;limit
	MOV	[ESP+6],AX
	LIDT	[ESP+6]
	RET

asm_inthandler21	:	;void asm_inthandler21
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

asm_inthandler27	:	;void asm_inthandler27
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

asm_inthandler2c	:	;void inthandler2c
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

