#include<stdio.h>
#include"bootpack.h"

unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);

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

    init_gdtidt();
    init_pic();
    _io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
    

    fifo8_init(&keyfifo,32,keybuf);
    fifo8_init(&mousefifo,128,mousebuf);

    _io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) ここでは後ろから数値が入るつまり（10011111）の順番ではいる */
    _io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111)ここでも(11110111)で入るこれは0x2cに値する */

    init_keyboard();
    
    _init_palette();
    init_screen(binfo->vram,binfo->scrnx,binfo->scrny);
    

    init_mouse_cursor8(mcursor,COL8_008484);
    putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,mcursor,16);
    sprintf(s,"(%d,%d)",mx,my);
    putfont8_asc(binfo->vram,binfo->scrnx,0,0,COL8_FFFFFF,s);

    enable_mouse(&mdec);

    i=memtest(0x00400000,0xbfffffff)/(1024*1024);
    sprintf(s,"memory %dMB",i);
    putfont8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

    

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


#define EFLAGS_AC_BIT       0x00040000
#define CR0_CACHE_DISABLE   0x60000000


//このプログラムはキャッシュ機能をOFFにしている(これをしないとメモリチェックできない)
unsigned int memtest(unsigned int start,unsigned int end){
    char flg486=0;
    unsigned int eflg,cr0,i;

    /*386 か　486以降なのか確認*/
    eflg =_io_load_eflags();/*フラグ(eflag)の状態を確認するためにフラグの値を取り出す関数*/

    eflg|=EFLAGS_AC_BIT; /*ACフラグ(EFLAGSの18bit)というものを1にする。これはそれぞれ異なっている要素*/
    _io_store_eflags(eflg);/*フラグ（ここでは`1`になった状態）を代入する*/
    eflg=_io_load_eflags();
    if((eflg&EFLAGS_AC_BIT)!=0){/*386だと自然に０に戻ってしまう*/
        flg486=1;
    }
    eflg&=~EFLAGS_AC_BIT;/*AC_BIT=0*/
    _io_store_eflags(eflg);/*ここで0に戻している*/

    if(flg486!=0){
        cr0=load_cr0();
        cr0|=CR0_CACHE_DISABLE;/*キャッシュ禁止*/
        store_cr0(cr0);
    }

    i=memtest_sub(start,end);/*ここでメモリの量を確認するメモリチェック*/

    if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* キャッシュ許可 */
		store_cr0(cr0);
	}

	return i;
}

//メモリをチェックする関数
unsigned int memtest_sub(unsigned int start, unsigned int end){
    unsigned int i, *p,old, pat0=0xaa55aa55,pat1=0x55aa55aa;
    for(i=start;i<=end;i+=0x1000){
        /*4byteだと遅いので4KBずつ調べる。ここでは0xffcは一番下から4Byteをチェック*/
        p=(unsigned int *)(i+0xffc);
        old=*p;     /*いじる前の値を記憶しておく*/

        *p=pat0;    /*メモリの中に試しに書いてみる*/
        *p^=0xffffffff;     /*メモリの中身を反転してみる*/
        if(*p!=pat1){
            /*反転結果になったかを確認する*/
            not_memory:
            *p=old;
            break;
        }
        *p^=0xffffffff; /*もう一度反転させる*/
        if(*p!=pat0){
            goto not_memory;

        }
        *p=old;/*元に戻す*/
    }
    return i;
}
