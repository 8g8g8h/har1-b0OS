;har1b0
;TAB=4

;BOOTの情報(16bitmodeでBIOS設定の最後)
CYLS	EQU	0x0ff0		;ブートセクタが設定する（使われてなかったから使っただけ）
LEDS	EQU	0x0ff1
VMODE	EQU	0x0ff2		;色数に関する情報何ビットカラーか
SCRNX	EQU	0x0ff4		;画面のx座標の解像度
SCRNY	EQU	0x0ff6		;画面のy座標の解像度
VRAM	EQU	0x0ff8		;グラフィックバッファの開始番地

	ORG	0xc200		;このプログラムが読み込まれる場所(0x800 から始まり、ファイルの中身は0x4200を足した場所なのでそれらを足した数字)
	
	MOV	AL,0x13		;VGAグラフィックス、320x200x8bitカラー
	MOV	AH,0x00
	INT	0x10
	MOV	BYTE [VMODE],8	;画面モードをメモする
	MOV 	WORD [SCRNX],320
	MOV	WORD [SCRNX],200
	MOV	DWORD [VRAM],0x000a000

;キーボードのLED状態をBIOSから教えてもらう

	MOV	AH,0x02
	INT	0x16		;キーボードのBIOS情報
	MOV	[LEDS],AL	
fin:

	HLT
	JMP	fin
