#include<stdio.h>
#include"bootpack.h"

void HariMain(void){
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    char s[40];	/*デバッグに必要な変数*/
    char mcursor[256]; /*マウスを表記するための変数*//*マウスに必要な変数*/
    char keybuf[32];
    char mousebuf[128];
    int mx=(binfo->scrnx-16)/2; /*マウスのｘ座標*/
    int my=(binfo->scrny-28-16)/2; /*マウスのｙ座標*/
    int i;
    struct MOUSE_DEC mdec;

    unsigned int memtotal;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

    init_gdtidt();
    init_pic();
    _io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
    

    fifo8_init(&keyfifo,32,keybuf);
    fifo8_init(&mousefifo,128,mousebuf);

    _io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) ここでは後ろから数値が入るつまり（10011111）の順番ではいる */
    _io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111)ここでも(11110111)で入るこれは0x2cに値する */

    init_keyboard();
    enable_mouse(&mdec);

    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    
    _init_palette();
    init_screen(binfo->vram,binfo->scrnx,binfo->scrny);
    

    init_mouse_cursor8(mcursor,COL8_008484);
    putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,mcursor,16);
    sprintf(s,"(%d,%d)",mx,my);
    putfont8_asc(binfo->vram,binfo->scrnx,0,0,COL8_FFFFFF,s);

    sprintf(s, "memory %dMB   free : %dKB",memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfont8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

    sprintf(s,"memman->frees %d",memman->frees);
    putfont8_asc(binfo->vram,binfo->scrnx,64,128,COL8_FFFFFF,s);

    for(;;){
        _io_cli(); /*割り込み禁止*/
        if(fifo8_status(&keyfifo)+fifo8_status(&mousefifo)== 0){
            _io_stihlt(); /*割り込み禁止を解除しながらhlt(cpuを待機状態にする)*/
        } else {
            if(fifo8_status(&keyfifo)!=0){
                i=fifo8_get(&keyfifo);
                _io_sti();/*CPUの割り込み禁止解除*/

                sprintf(s, "%x",i);
			    bookfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
			    putfont8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            }else if(fifo8_status(&mousefifo)!=0){
                i=fifo8_get(&mousefifo);
                _io_sti();/*CPUの割り込み禁止解除*/
                if(mouse_decode(&mdec,i)!=0){
                    
                    /*マウスカーソルの移動*/
                    bookfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx+15, my+15);/*マウスを消す*/
                    mx+=mdec.x;
                    my+=mdec.y;
                    if(mx < 0){
                        mx=0;
                    }
                    if(my < 0){
                        my=0;
                    }
                    if(mx > binfo->scrnx-16){
                        mx=binfo->scrnx-16;
                    }
                    if(my > binfo->scrny-16){
                        my=binfo->scrny-16;
                    }
                    sprintf(s,"(%d,%d)",mx,my);
                    bookfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);/*座標消す*/
                    putfont8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 座標書く */
                    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* マウス描く */
                }                
            }
        }
    }
}

