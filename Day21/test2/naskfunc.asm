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
  	GLOBAL  farjmp
  	GLOBAL  asm_hrb_api
	GLOBAL	start_app
  	GLOBAL  farcall 
  	EXTERN  inthandler20, inthandler21, inthandler27, inthandler2c
  	EXTERN  hrb_api
  	
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

asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OSが動いているときに割り込まれたのでほぼ今までどおり
		MOV		EAX,ESP
		PUSH	SS				; 割り込まれたときのSSを保存
		PUSH	EAX				; 割り込まれたときのESPを保存
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler20
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	アプリが動いているときに割り込まれた
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 割り込まれたときのSSを保存
		MOV		[ECX  ],ESP		; 割り込まれたときのESPを保存
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	inthandler20
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SSをアプリ用に戻す
		MOV		ESP,ECX			; ESPもアプリ用に戻す
		POPAD
		POP		DS
		POP		ES
		IRETD

asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OSが動いているときに割り込まれたのでほぼ今までどおり
		MOV		EAX,ESP
		PUSH	SS				; 割り込まれたときのSSを保存
		PUSH	EAX				; 割り込まれたときのESPを保存
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler21
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	アプリが動いているときに割り込まれた
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 割り込まれたときのSSを保存
		MOV		[ECX  ],ESP		; 割り込まれたときのESPを保存
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	inthandler21
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SSをアプリ用に戻す
		MOV		ESP,ECX			; ESPもアプリ用に戻す
		POPAD
		POP		DS
		POP		ES
		IRETD

asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OSが動いているときに割り込まれたのでほぼ今までどおり
		MOV		EAX,ESP
		PUSH	SS				; 割り込まれたときのSSを保存
		PUSH	EAX				; 割り込まれたときのESPを保存
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler27
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	アプリが動いているときに割り込まれた
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 割り込まれたときのSSを保存
		MOV		[ECX  ],ESP		; 割り込まれたときのESPを保存
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	inthandler27
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SSをアプリ用に戻す
		MOV		ESP,ECX			; ESPもアプリ用に戻す
		POPAD
		POP		DS
		POP		ES
		IRETD

asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OSが動いているときに割り込まれたのでほぼ今までどおり
		MOV		EAX,ESP
		PUSH	SS				; 割り込まれたときのSSを保存
		PUSH	EAX				; 割り込まれたときのESPを保存
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler2c
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	アプリが動いているときに割り込まれた
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 割り込まれたときのSSを保存
		MOV		[ECX  ],ESP		; 割り込まれたときのESPを保存
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	inthandler2c
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SSをアプリ用に戻す
		MOV		ESP,ECX			; ESPもアプリ用に戻す
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

farjmp:    ;void farjmp(int rip,int cs);
    JMP FAR [ESP+4]
    RET

farcall: ;void farcall(int eip,int cs); これは、farjmpと同じようなcallである。これにより、retfで関数に返ってくることができる
    CALL  FAR [ESP+4]
    RET

asm_hrb_api:
    		; 都合のいいことに最初から割り込み禁止になっている
		PUSH	DS
		PUSH	ES
		PUSHAD		; 保存のためのPUSH
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-40
		MOV		[ECX+32],ESP	; アプリのESPを保存
		MOV		[ECX+36],SS		; アプリのSSを保存

; PUSHADした値をシステムのスタックにコピーする
		MOV		EDX,[ESP   ]
		MOV		EBX,[ESP+ 4]
		MOV		[ECX   ],EDX	; hrb_apiに渡すためコピー
		MOV		[ECX+ 4],EBX	; hrb_apiに渡すためコピー
		MOV		EDX,[ESP+ 8]
		MOV		EBX,[ESP+12]
		MOV		[ECX+ 8],EDX	; hrb_apiに渡すためコピー
		MOV		[ECX+12],EBX	; hrb_apiに渡すためコピー
		MOV		EDX,[ESP+16]
		MOV		EBX,[ESP+20]
		MOV		[ECX+16],EDX	; hrb_apiに渡すためコピー
		MOV		[ECX+20],EBX	; hrb_apiに渡すためコピー
		MOV		EDX,[ESP+24]
		MOV		EBX,[ESP+28]
		MOV		[ECX+24],EDX	; hrb_apiに渡すためコピー
		MOV		[ECX+28],EBX	; hrb_apiに渡すためコピー

		MOV		ES,AX			; 残りのセグメントレジスタもOS用にする
		MOV		SS,AX
		MOV		ESP,ECX
		STI			; やっと割り込み許可

		CALL	hrb_api

		MOV		ECX,[ESP+32]	; アプリのESPを思い出す
		MOV		EAX,[ESP+36]	; アプリのSSを思い出す
		CLI
		MOV		SS,AX
		MOV		ESP,ECX
		POPAD
		POP		ES
		POP		DS
		IRETD		; この命令が自動でSTIしてくれる

start_app :      ;void start_app(int eip,int cs,int esp,int ds)
  PUSHAD		; 32ビットレジスタを全部保存しておく
		MOV		EAX,[ESP+36]	; アプリ用のEIP
		MOV		ECX,[ESP+40]	; アプリ用のCS
		MOV		EDX,[ESP+44]	; アプリ用のESP
		MOV		EBX,[ESP+48]	; アプリ用のDS/SS
		MOV		[0xfe4],ESP		; OS用のESP
		CLI			; 切り替え中に割り込みが起きてほしくないので禁止
		MOV		ES,BX
		MOV		SS,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
		MOV		ESP,EDX
		STI			; 切り替え完了なので割り込み可能に戻す
		PUSH	ECX				; far-CALLのためにPUSH（cs）
		PUSH	EAX				; far-CALLのためにPUSH（eip）
		CALL	FAR [ESP]		; アプリを呼び出す

;	アプリが終了するとここに帰ってくる

		MOV		EAX,1*8			; OS用のDS/SS
		CLI			; また切り替えるので割り込み禁止
		MOV		ES,AX
		MOV		SS,AX
		MOV		DS,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		ESP,[0xfe4]
		STI			; 切り替え完了なので割り込み可能に戻す
		POPAD	; 保存しておいたレジスタを回復
		RET
