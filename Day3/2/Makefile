#Makefile

default:
	make img

ipl.bin	:ipl.asm Makefile
	nasm ipl.asm -o ipl.bin -l ipl.lst

har1b0.sys :har1b0.asm Makefile
	nasm har1b0.asm -o har1b0.sys -l har1b0.ipl

har1b0.img : ipl.bin har1b0.sys Makefile
	mformat -f 1440 -C -B ipl.bin -i har1b0.img ::
	mcopy har1b0.sys -i har1b0.img ::

asm : 
	make -r ipl.bin

img :	
	make -r har1b0.img

run :
	make img 
	qemu-system-i386 -fda har1b0.img #"-fda" for floppy disk


#コメント
# sysでディスクイメージに必要なものを準備
#その後imgファイルを作成
#ここでimgの0x2600以降にファイル名が入る。また0x004200以降にファイルの中身が入る
#つまりファイルの内容が↑の場所に入る
