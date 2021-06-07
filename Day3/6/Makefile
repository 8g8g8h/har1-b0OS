#Makefile

default:
	make img

ipl10.bin : ipl10.asm Makefile
	nasm ipl10.asm -o ipl10.bin -l ipl10.lst

asmhead.bin : asmhead.asm Makefile
	nasm asmhead.asm -o asmhead.bin -l asmhead.lst

naskfunc.o : naskfunc.asm Makefile
	nasm -g -f elf naskfunc.asm -o naskfunc.o 

bootpack.o : bootpack.c 
	gcc -c -m32 -o bootpack.o bootpack.c -fno-pic

bootpack.bin : bootpack.o os.lds naskfunc.o Makefile	
	ld -m elf_i386 -e HariMain -o bootpack.bin -T os.lds bootpack.o naskfunc.o 

har1b0.sys :asmhead.bin bootpack.bin Makefile
	cat asmhead.bin bootpack.bin> har1b0.sys

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

debug:
	make img 
	qemu-system-i386 -fda har1b0.img -gdb tcp::10000 -S

clean:
	rm *.lst *.bin *.sys *.img *.hrb 



