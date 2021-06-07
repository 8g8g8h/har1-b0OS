#Makefile

default:
	make img

conHankaku : conHankaku.c
	gcc conHankaku.c -o conHankaku

hankaku.c :hankaku.txt conHankaku
	./conHankaku

hankaku.o : hankaku.c
	gcc -c -m32  -o hankaku.o hankaku.c 

ipl10.bin : ipl10.asm Makefile
	nasm ipl10.asm -o ipl10.bin -l ipl10.lst

asmhead.bin : asmhead.asm Makefile
	nasm asmhead.asm -o asmhead.bin -l asmhead.lst

naskfunc.o : naskfunc.asm Makefile
	nasm -g -f elf naskfunc.asm -o naskfunc.o 

bootpack.o: bootpack.c Makefile
	gcc -fno-pic -c -m32 -o bootpack.o bootpack.c

hello.hrb: hello.asm Makefile 
	nasm -o hello.hrb hello.asm 

hello2.hrb: hello2.asm Makefile
	nasm -o hello2.hrb	hello2.asm

a.o : a.c Makefile
	gcc -fno-pic -c -m32 -o a.o a.c

a_nasm.o : a_nasm.asm Makefile
	nasm -f elf32 -o a_nasm.o a_nasm.asm

#a.hrb : a.o a_nasm.o Makefile
#	ld -m elf_i386 -e HariMain -o a.hrb -T api.lds a_nasm.o a.o

hello3.o:hello3.c Makefile
	gcc -fno-pic -c -m32 -o hello3.o hello3.c

hello3.hrb :hello3.o a_nasm.o Makefile
	ld -m elf_i386 -e HariMain -o hello3.hrb -T api.lds hello3.o a_nasm.o

crack1.o: crack1.c Makefile
	gcc -fno-pic -c -m32 -o crack1.o crack1.c

crack1.hrb: crack1.o Makefile
	ld -m elf_i386 -e HariMain -o crack1.hrb -T api.lds crack1.o 

bootpack.bin : bootpack.o naskfunc.o hankaku.o myprintf.o graphics.o console.o dsctbl.o int.o fifo.o file.o keyboard.o mouse.o memory.o sheets.o str.o timer.o mtask.o window.o Makefile
	ld -Map=bootpack.map -m elf_i386 -e HariMain -o bootpack.bin -Tos.lds bootpack.o naskfunc.o hankaku.o myprintf.o graphics.o console.o dsctbl.o int.o fifo.o file.o keyboard.o mouse.o memory.o sheets.o str.o timer.o mtask.o window.o 

har1b0.sys :asmhead.bin bootpack.bin Makefile
	cat asmhead.bin bootpack.bin> har1b0.sys

har1b0.img : ipl10.bin har1b0.sys hello.hrb hello2.hrb a.hrb hello3.hrb crack1.hrb Makefile
	mformat -f 1440 -C -B ipl10.bin -i har1b0.img ::
	mcopy har1b0.sys -i har1b0.img ::
	mcopy hello.hrb -i har1b0.img ::
	mcopy hello2.hrb -i har1b0.img ::
	mcopy hello3.hrb -i har1b0.img ::
	mcopy a.hrb	-i har1b0.img ::
	mcopy	crack1.hrb -i har1b0.img ::
%.o : %.c
	gcc -fno-pic -c -m32 -nostdlib $*.c -o $*.o

asm : 
	make -r ipl10.bin

img :	
	make -r har1b0.img

run :
	make img 
	qemu-system-i386 -m 32 -fda har1b0.img -enable-kvm #"-fda" for floppy disk

debug:
	make img 
	qemu-system-i386 -fda har1b0.img -gdb tcp::10000 -S

clean:
	rm *.lst *.bin *.sys *.img *.hrb 






