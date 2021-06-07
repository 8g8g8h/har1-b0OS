#Makefile

ipl10.bin : ipl10.asm 
	nasm ipl10.asm -o ipl10.bin -l ipl10.lst

asmhead.bin: asmhead.asm 
	nasm asmhead.asm -o asmhead.bin -l asmhead.lst

nasmfunc.o : nasmfunc.asm 
	nasm -f elf nasmfunc.asm -o nasmfunc.o -l nasmfunc.lst

bootpack.o : bootpack.c 
	gcc -c -m32 -fno-pic -o bootpack.o bootpack.c -l bootpack.lst

bootpack.bin : bootpack.o nasmfunc.o 
	 ld -m elf_i386 -e HariMain -o bootpack.bin -Tos.ls bootpack.o nasmfunc.o 

har1b0.sys : asmhead.bin bootpack.bin 
	cat asmhead.bin bootpack.bin > har1b0.sys

har1b0.img : ipl10.bin har1b0.sys 
	mformat -f 1440 -C -B ipl10.bin	-i har1b0.img ::
	mcopy har1b0.sys -i har1b0.img ::

run: har1b0.img
	qemu-system-i386 -fda har1b0.img

clean:
	rm -f *.bin
	rm -f *.o
	rm -f *.sys
	rm -f *.img

debug: 
	qemu-system-i386 -m 32 -localtime -vga std -fda har1b0.img -gdb tcp::10000 -S &
