;naskfunc.asm
;TAB=4
; このプログラムに含まれる関数名
	GLOBAL	_io_hlt,_io_cli,_io_sti,_io_stihlt
	GLOBAL	_io_in8,_io_in16,_io_in32
	GLOBAL	_io_out8,_io_out16,_io_out32
	GLOBAL	_io_load_eflags,_io_store_eflags
	GLOBAL  load_gdtr,load_idtr,load_tr
	GLOBAL	asm_inthandler20,asm_inthandler21, asm_inthandler27, asm_inthandler2c,asm_inthandler0c,asm_inthandler0d
	GLOBAL	load_cr0,store_cr0
	GLOBAL	memtest_sub
  	GLOBAL  farjmp
  	GLOBAL  asm_hrb_api
	GLOBAL	start_app,asm_end_app
  	GLOBAL  farcall 
  	EXTERN  inthandler20, inthandler21, inthandler27, inthandler2c,inthandler0d,inthandler0c
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

asm_inthandler20	:	;void asm_inthandler20
    PUSH	ES
		PUSH	DS
		PUSHAD      ;ここでは、どちらのスタックが使われていても、最終的に戻ってきたときにもとの状態にしないといけないので、すべてのレジスタやセグメントレジスタなどを保存しておく必要がある
    MOV   EAX,ESP
    PUSH  EAX
    MOV   AX,SS   
		MOV		DS,AX   
		MOV		ES,AX
		CALL	inthandler20
    POP   EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

asm_inthandler21	:	;void asm_inthandler21
    PUSH	ES
		PUSH	DS
		PUSHAD      ;ここでは、どちらのスタックが使われていても、最終的に戻ってきたときにもとの状態にしないといけないので、すべてのレジスタやセグメントレジスタなどを保存しておく必要がある
		MOV		EAX,ESP
		PUSH	EAX       
		MOV		AX,SS   
		MOV		DS,AX   
		MOV		ES,AX
		CALL	inthandler21
    POP   EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

asm_inthandler27	:
    PUSH	ES
		PUSH	DS
		PUSHAD      ;ここでは、どちらのスタックが使われていても、最終的に戻ってきたときにもとの状態にしないといけないので、すべてのレジスタやセグメントレジスタなどを保存しておく必要がある
		MOV		EAX,ESP
		PUSH	EAX       ;割り込まれたときのESPを保持
		MOV		AX,SS   
		MOV		DS,AX   
		MOV		ES,AX
		CALL	inthandler27
    POP   EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

asm_inthandler2c	:
    PUSH	ES
		PUSH	DS
		PUSHAD      ;ここでは、どちらのスタックが使われていても、最終的に戻ってきたときにもとの状態にしないといけないので、すべてのレジスタやセグメントレジスタなどを保存しておく必要がある
    MOV		EAX,ESP
		PUSH	EAX       ;割り込まれたときのESPを保持
		MOV		AX,SS   
		MOV		DS,AX   
		MOV		ES,AX
		CALL	inthandler2c
    POP   EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
asm_inthandler0c  : ;void asm_inthandler0c
    STI
    PUSH	ES
		PUSH	DS
		PUSHAD      ;ここでは、どちらのスタックが使われていても、最終的に戻ってきたときにもとの状態にしないといけないので、すべてのレジスタやセグメントレジスタなどを保存しておく必要がある
		MOV		EAX,ESP
		PUSH	EAX       ;割り込まれたときのESPを保持
		MOV		AX,SS   
		MOV		DS,AX   
		MOV		ES,AX
		CALL	inthandler0c
    CMP   EAX,0
    JNE   asm_end_app
    POP   EAX
		POPAD
		POP		DS
		POP		ES
    ADD   ESP,4     ;INT 0x0cでは、これが必要
		IRETD   

asm_inthandler0d	:	;void asm_inthandler0d
    STI
    PUSH	ES
		PUSH	DS
		PUSHAD      ;ここでは、どちらのスタックが使われていても、最終的に戻ってきたときにもとの状態にしないといけないので、すべてのレジスタやセグメントレジスタなどを保存しておく必要がある
		MOV		EAX,ESP
		PUSH	EAX       ;割り込まれたときのESPを保持
		MOV		AX,SS   
		MOV		DS,AX   
		MOV		ES,AX
		CALL	inthandler0d
    CMP   EAX,0
    JNE   asm_end_app
    POP   EAX
		POPAD
		POP		DS
		POP		ES
    ADD   ESP,4     ;INT 0x0dでは、これが必要
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
    ;最初から割り込み禁止になっている(start_appで設定していた)
    STI
    PUSH  DS      ;アプリのDS
    PUSH  ES      ;アプリのES 
    ;ここで、アプリのスタックなどに切り替えている。
    PUSHAD  ;保存のためのPUSH(今のレジスタの内容(アプリのための設定が入っている)を保存しておく) 
    PUSHAD  ;hrb_api(引数をhrb_apiに渡すためにスタックに積む)が使う値を渡す(ここで引数を渡したらその呼び出し関数でこのPUSHADしている値を全部popしている)
    ;ここでカーネルを呼び出すのでセグメントを切り替えている。
    ;今までは、自分でデータを送らないと行けないので1つのスタックを使ってデータを移動していたが、今回は、スタック切り替えは、cpuに行わせている
    MOV   AX,SS     ;OS用のセグメントが入っているSSをAXに入れている
    MOV   DS,AX     ;OS用のセグメントをDSとESにも入れる
    MOV   ES,AX
    CALL  hrb_api
    CMP   EAX,0     ;EAXが0でなければアプリ終了
    JNE   asm_end_app
    ADD   ESP,32
    POPAD         ;保存していたPUSHADの内容をレジスタに戻す
    POP   ES      ;アプリのESをレジスタに戻す
    POP   DS      ;アプリのDSをレジスタに戻す
    IRETD         ;アプリに戻る(割り込みから戻るのでINT0X40が呼び出されたところに戻るここでは、アプリ)

asm_end_app:
;EAXはtss.esp0の番地
    MOV   ESP,[EAX]
    MOV   DWORD [EAX+4],0
    POPAD
    RET             ;cmd_appへ帰る(osのほうに戻る)

start_app :      ;void start_app(int eip,int cs,int esp,int ds,int *tss_esp0)
  PUSHAD    ;32bitレジスタを全部保存しておく(アプリからOSに戻ってきたときに正常に動くためにアプリを行う前のレジスタをすべて保存しておく)
  MOV   EAX,[ESP+36]  ;アプリ用のEIP(esp+36なのは、事前のPUSHADでスタックにESP+32まで積んであるため)
  MOV   ECX,[ESP+40]  ;アプリ用のCS
  MOV   EDX,[ESP+44]  ;アプリ用のESP
  MOV   EBX,[ESP+48]  ;アプリ用のDS/SS
  MOV   EBP,[ESP+52]  ;tss.esp0の番地(OSのスタックを示すesp)
  MOV   [EBP],ESP     ;OS用のESPを保存
  MOV   [EBP+4],SS    ;OS用のSSを保存
  MOV   ES,BX         ;ここから同じものをいろいろなレジスタに入れているがこれは念のためであるらしい
  MOV   DS,BX
  MOV   FS,BX
  MOV   GS,BX
;以下はRETFでアプリに行かせるためのスタック調整,
;そしてpushされた値から番地を計算しretfでその番地に移動
  OR    ECX,3         ;アプリ用のセグメント番号に3をORする
  OR    EBX,3         ;アプリ用のセグメント番号に3をORする(ここで、RPLという特権レベルを最低にしておく。これにより、アプリケーション側からOSのセグメントにアクセスしようとしたときに例外が発生するようになっている。)
  PUSH  EBX           ;アプリのSS
  PUSH  EDX           ;アプリのESP
  PUSH	ECX	      ;アプリのcs
  PUSH  EAX           ;アプリのEIP
  RETF
;これで、アプリを展開しているメモリ番地(コードセグメントの番地)に移動する
;移動して、INT40が呼ばれてasm_hrb_apiに移動する
;アプリが終了してもここには来ない 
