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

bootpack.hrb : bootpack.c os.lds naskfunc.o hankaku.o myprintf.o graphics.o dsctbl.o int.o fifo.o  keyboard.o mouse.o  memory.o sheets.o timer.o mtask.o  Makefile	
	gcc -march=i486 -m32 -nostdlib -T os.lds -g bootpack.c  naskfunc.o hankaku.o myprintf.o graphics.o dsctbl.o int.o  fifo.o keyboard.o mouse.o memory.o sheets.o timer.o mtask.o -o bootpack.hrb -fno-pic

har1b0.sys :asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb> har1b0.sys

har1b0.img : ipl10.bin har1b0.sys Makefile
	mformat -f 1440 -C -B ipl10.bin -i har1b0.img ::
	mcopy har1b0.sys -i har1b0.img ::
	mcopy fifo.c -i har1b0.img ::
%.o : %.c
	gcc -c -m32 -fno-pic -nostdlib $*.c -o $*.o

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






