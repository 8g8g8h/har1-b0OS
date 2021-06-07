;har1b0
;TAB=4
;OSプログラム本体

	ORG	0xc200		;このプログラムが読み込まれる場所(0x8000から始まり、ファイルの中身は0x4200を足した場所から始まる。それらを足した数字)
	
	MOV	AL,0x13		;VGAグラフィックス、320x200x8bitカラー
	MOV	AH,0x00
	INT	0x10
fin:

	HLT
	JMP	fin
