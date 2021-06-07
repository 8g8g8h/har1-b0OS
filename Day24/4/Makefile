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

a.hrb : a.o a_nasm.o Makefile
	ld -m elf_i386 -e HariMain -o a.hrb -T api.lds a_nasm.o a.o

hello3.o:hello3.c Makefile
	gcc -nostdlib -fno-pic -c -m32 -o hello3.o hello3.c

hello3.hrb :hello3.o a_nasm.o Makefile
	ld -m elf_i386 -e HariMain -o hello3.hrb -T api.lds a_nasm.o hello3.o

hello4.o:hello4.c Makefile
	gcc -fno-pic -c -m32 -o hello4.o hello4.c

hello4.hrb:hello4.o a_nasm.o Makefile
	ld -m elf_i386 -e HariMain -o hello4.hrb -T api.lds hello4.o a_nasm.o

hello5.o: hello5.asm Makefile
	nasm -f elf32 -o hello5.o hello5.asm

hello5.hrb:hello5.o Makefile
	ld -m elf_i386 -e HariMain -o hello5.hrb -T api.lds hello5.o

bug1.o: bug1.c Makefile
	gcc -nostdlib -fno-pic -c -m32 -o bug1.o bug1.c 

bug1.hrb: bug1.o a_nasm.o Makefile
	ld -Map=bug1.map -m elf_i386 -e HariMain -o bug1.hrb -T api.lds  bug1.o a_nasm.o 

bug2.hrb:bug2.c Makefile
	gcc -c -m32 -o bug2.hrb bug2.c	

bug3.o:bug3.c Makefile
	gcc -nostdlib -fno-pic -c -m32 -o bug3.o bug3.c

bug3.hrb:bug3.o a_nasm.o Makefile
	ld -Map=bug3.map -m elf_i386 -e HariMain -o bug3.hrb -T api.lds bug3.o a_nasm.o

winhelo.o:winhelo.c Makefile
	gcc -fno-pic -c -m32 -o winhelo.o winhelo.c

winhelo.hrb:winhelo.o a_nasm.o Makefile
	ld -Map=winhelo.map -m elf_i386 -e HariMain -o winhelo.hrb -T api.lds winhelo.o a_nasm.o

winhelo2.o:winhelo2.c Makefile
	gcc -fno-pic -c -m32 -o winhelo2.o winhelo2.c

winhelo2.hrb:winhelo2.o a_nasm.o Makefile
	ld -m elf_i386 -e HariMain -o winhelo2.hrb -T api.lds winhelo2.o a_nasm.o

winhelo3.o: winhelo3.c Makefile
	gcc -fno-pic -c -m32 -o winhelo3.o winhelo3.c

winhelo3.hrb:winhelo3.o a_nasm.o Makefile
	ld -m elf_i386 -e HariMain -o winhelo3.hrb -T api.lds winhelo3.o a_nasm.o

star1.o:star1.c Makefile
	gcc -fno-pic -c -m32 -o star1.o star1.c

star1.hrb:star1.o a_nasm.o Makefile
	ld -m elf_i386 -e HariMain -o star1.hrb -T api.lds star1.o a_nasm.o

stars.o:stars.c Makefile
	gcc -fno-pic -c -m32 -o stars.o stars.c

stars.hrb:stars.o a_nasm.o Makefile
	ld -m elf_i386 -e HariMain -o stars.hrb -T api.lds stars.o a_nasm.o

stars2.o:stars2.c Makefile
	gcc -fno-pic -c -m32 -o stars2.o stars2.c

stars2.hrb:stars2.o a_nasm.o Makefile
	ld -m elf_i386 -e HariMain -o stars2.hrb -T api.lds stars2.o a_nasm.o

lines.o:lines.c Makefile
	gcc -fno-pic -c -m32 -o lines.o lines.c

lines.hrb:lines.o a_nasm.o Makefile
	ld -m elf_i386 -e HariMain -o lines.hrb -T api.lds lines.o a_nasm.o

walk.o:walk.c Makefile
	gcc -fno-pic -c -m32 -o walk.o walk.c

walk.hrb:walk.o a_nasm.asm Makefile
	ld -m elf_i386 -e HariMain -o walk.hrb -T api.lds walk.o a_nasm.o

bootpack.bin : bootpack.o naskfunc.o hankaku.o myprintf.o graphics.o console.o dsctbl.o int.o fifo.o file.o keyboard.o mouse.o memory.o sheets.o str.o timer.o mtask.o window.o Makefile
	ld -Map=bootpack.map -m elf_i386 -e HariMain -o bootpack.bin -Tos.lds bootpack.o naskfunc.o hankaku.o myprintf.o graphics.o console.o dsctbl.o int.o fifo.o file.o keyboard.o mouse.o memory.o sheets.o str.o timer.o mtask.o window.o 

har1b0.sys :asmhead.bin bootpack.bin Makefile
	cat asmhead.bin bootpack.bin> har1b0.sys

har1b0.img : ipl10.bin har1b0.sys hello.hrb hello2.hrb a.hrb hello3.hrb bug1.hrb bug2.hrb bug3.hrb hello4.hrb hello5.hrb winhelo.hrb winhelo2.hrb winhelo3.hrb star1.hrb stars.hrb stars2.hrb lines.hrb walk.hrb  Makefile
	mformat -f 1440 -C -B ipl10.bin -i har1b0.img ::
	mcopy har1b0.sys -i har1b0.img ::
	mcopy hello.hrb -i har1b0.img ::
	mcopy hello2.hrb -i har1b0.img ::
	mcopy hello3.hrb -i har1b0.img ::
	mcopy a.hrb	-i har1b0.img ::
	mcopy	bug1.hrb -i har1b0.img ::
	mcopy bug2.hrb -i har1b0.img ::
	mcopy bug3.hrb -i har1b0.img ::
	mcopy hello4.hrb -i har1b0.img ::
	mcopy hello5.hrb -i har1b0.img ::
	mcopy winhelo.hrb -i har1b0.img ::
	mcopy	winhelo2.hrb -i har1b0.img ::
	mcopy winhelo3.hrb -i har1b0.img ::
	mcopy star1.hrb -i har1b0.img ::
	mcopy stars.hrb -i har1b0.img ::
	mcopy stars2.hrb -i har1b0.img ::
	mcopy lines.hrb -i har1b0.img ::
	mcopy walk.hrb -i har1b0.img ::

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






