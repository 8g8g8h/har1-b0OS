#Makefile

default:
	make img

ipl10.bin : ipl10.asm Makefile
	nasm ipl10.asm -o ipl10.bin -l ipl10.lst

har1b0.sys :har1b0.asm Makefile
	nasm har1b0.asm -o har1b0.sys -l har1b0.lst

har1b0.img : ipl10.bin har1b0.sys Makefile
	mformat -f 1440 -C -B ipl10.bin -i har1b0.img ::
	mcopy har1b0.sys -i har1b0.img ::

asm : 
	make -r ipl10.bin

img :	
	make -r har1b0.img

run :
	make img 
	qemu-system-i386 -fda har1b0.img #"-fda" for floppy disk



