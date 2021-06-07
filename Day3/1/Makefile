.DEFAULT_GOAL : all
.PHONY : all
all : img

ipl.bin : ipl.asm

%.bin : %.asm
	nasm $^ -o $@ -l $*.lst

har1b0-OS.img : ipl.bin
	cat $^ > $@


.PHONY : asm
asm :
	make ipl.bin

.PHONY : img
img :
	make har1b0-OS.img

.PHONY : run
run :
	make img
	qemu-system-i386 -fda har1b0-OS.img

.PHONY : clean
clean :
# lstは残しておいてもいいと思うのでcleanに入れていない
		@rm *.img *.bin


