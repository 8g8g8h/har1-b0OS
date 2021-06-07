;har1b0OS-ipl
; TAB=4

CYLS		EQU		10
		ORG		0x7c00		;このプログラムがどこに読み込まれるのか

;以下は標準的なFAT12フォーマットフロッピーディスクのための記述

		JMP		entry
		DB		0x90
		DB		"HAR1-B0ipl"		;ブートセクタの名前
		DW		512				;1セクタの大きさ
		DB		1				;FAT開始場所
		DW		1				;
		DB		2				;FATの個数
		DW		224				;ルートディレクトリ領域の大きさ
		DW		2880			;ドライブの大きさ
		DB		0xf0			;メディアタイプ
		DW		9				;FAT領域の長さ
		DW		18				;1トラックにあるセクタの数
		DW		2				;ヘッド数
		DD		0				;パーティション
		DB		0,0,0x29		;この値にする
		DD		0xffffffff		;ボリュームシリアル番号
		DB		"HAR1-B0OS"		;ディスクの名前
		DB		"FAT12	"		;フォーマットの名前
		RESB	18				;仮にあけとく

;プログラム本体

entry:
		MOV		AX,0			;レジスタ初期化
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

;ディスクを読む

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			;シリンダ0
		MOV		DH,0			;ヘッド0
		MOV		CL,2			;セクタ2

readloop:
		MOV		SI,0			;失敗回数を数えるレジスタ

retry:
		MOV		AH,0x02			;AH=0x02 :ディスク読み込み
		MOV		AL,1			;1セクタ
		MOV		BX,0			
		MOV		DL,0x00			;Aドライブ
		INT		0x13			;ディスクBIOS呼び出し
		JNC		next				;エラーにならなかったらfinへ
		ADD		SI,1			;SIに1を足す
		CMP		SI,5			;SIと5を比較
		JAE		error			;SI>=5だったらerrorへ
		MOV		AH,0x00			;(システムリセットに必要な操作)
		MOV		DL,0x00			;Aドライブ(システムリセットに必要な操作)
		INT		0x13			;ドライブのリセット(システムリセットに必要な操作)
		JMP		retry

next:
		MOV		AX,ES			;開始位置(0x820)まで移動する
		ADD		AX,0x0020		;アドレスを0x200進める
		MOV		ES,AX			;ADD ES,0x20という命令がないのでこのように表記
;シリンダ18まで
		ADD		CL,1			;CLに1を足す
		CMP		CL,18			;CLと18を比較
		JBE		readloop		;CL<=18だったらreadloopへ
;ヘッドの裏まで読んだのか確認
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		;DH<2だったらreadloopへ
;初期化
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop		;CH<CYLSだったらreadloop

;ブートローダ終了して、OSを読み込む

		MOV		[0x0ff0],CH		;IPLがどこまで読んだのかをsysに教えてあげるために情報が入っているCHをメモリの中に代入
		JMP		0xc200

error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			;SIに1を足す
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			;一文字表示ファンクション
		MOV		BX,15			;カラーコード
		INT		0x10			;ビデオBIOS呼び出し
		JMP		putloop

fin:
		HLT						;何かあるまでCPUを停止させる
		JMP		fin				;無限ループ

msg:
		DB		0x0a,0x0a		;改行を2つ
		DB		"load error"
		DB		0x0a			;改行
		DB		0

		RESB	0x7dfe-0x7c00-($-$$)		;0x1efまでを0x00で埋める命令

;end

		DB		0x55,0xaa		;BS BootSign
