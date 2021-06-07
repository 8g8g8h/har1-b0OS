;naskfunc.asm
;TAB=4
; このプログラムに含まれる関数名
	GLOBAL	_io_hlt,_io_cli,_io_sti,_io_stihlt
	GLOBAL	_io_in8,_io_in16,_io_in32
	GLOBAL	_io_out8,_io_out16,_io_out32
	GLOBAL	_io_load_eflags,_io_store_eflags
	GLOBAL  load_gdtr,load_idtr,load_tr
	GLOBAL	asm_inthandler20,asm_inthandler21, asm_inthandler27, asm_inthandler2c	
	GLOBAL	load_cr0,store_cr0
	GLOBAL	memtest_sub
  GLOBAL  taskswitch3,taskswitch4
	EXTERN	inthandler20, inthandler21, inthandler27, inthandler2c
  	
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

_io_out8	:	;int io_out8(int port,int data);

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

asm_inthandler20	:	;void asm_inthandler20
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

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

asm_inthandler27	:
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

asm_inthandler2c	:
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

load_cr0	:	;int load_cr0(void);
		MOV	EAX,CR0
		RET

store_cr0	:	;void store_cr0(int cr0);
		MOV	EAX,[ESP+4]
		MOV	CR0,EAX
		RET

memtest_sub	:	;unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI	;(EBX,ESI,EDIも使いたいため)
		PUSH	ESI
		PUSH	EBX
		MOV	ESI,0xaa55aa55	;pat0=0xaa55aa55
		MOV	EDI,0x55aa55aa	;pat1=0x55aa55aa
		MOV	EAX,[ESP+12+4]	;i=start;
mts_loop:
		MOV	EBX,EAX
		ADD	EBX,0xffc		;p=i+0xffc;
		MOV	EDX,[EBX]		;old=*p;
		MOV	[EBX],ESI		;*p=pat0;
		XOR	DWORD [EBX],0xffffffff	;*p^=0xffffffff;
		CMP	EDI,[EBX]		;if(*p!=pat1) goto fin
		JNE	mts_fin
		XOR	DWORD [EBX],0xffffffff	;*p^=0xffffffff;
		CMP	ESI,[EBX]		;if(*p!=pat0) goto fin
		JNE	mts_fin
		MOV	[EBX],EDX		;*p=old;
		ADD	EAX,0x1000		;i+=0x1000
		CMP	EAX,[ESP+12+8]		;if(i<=end)goto mts_loop;
		JBE	mts_loop
		POP	EBX
		POP 	ESI
		POP	EDI
		RET

mts_fin:
		MOV	[EBX],EDX		;*p=old
		POP	EBX
		POP	ESI
		POP	EDI
		RET

load_tr:  ;void load_tr(int tr);
    LTR [ESP+4] ;tr
    RET

taskswitch3:    ;void taskswitch3(void);
    JMP 3*8:0
    RET

taskswitch4:    ;void taskswitch4(void);
    JMP 4*8:0
    RET
