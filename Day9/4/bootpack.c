#include<stdio.h>
#include"bootpack.h"

#define MEMMAN_FREES    4090    /*これで約32KB*/
#define MEMMAN_ADDR 0x003c0000

struct FREEINFO{
    unsigned int addr,size;
};

struct MEMMAN
{
    int frees,maxfrees,lostsize,losts;/*frees=空きがあるリストの個数*/
    struct FREEINFO free[MEMMAN_FREES];/*メモリの空き情報を格納する情報の数。ここでは余裕を持って4000個用意している*/
};

unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memtest_sub(unsigned int start, unsigned int end);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man,unsigned int size);
int memman_free(struct MEMMAN *man,unsigned int addr,unsigned int size);


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

    sprintf(s,"memory->frees %d",memman->frees);
    putfont8_asc(binfo->vram, binfo->scrnx, 64,128,COL8_FFFFFF,s);

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

unsigned int memtest(unsigned int start,unsigned int end){
    char flg486=0;
    unsigned int eflg,cr0,i;

    /*386 か　486以降なのか確認*/
    eflg =_io_load_eflags();/*フラグ(eflag)の状態を確認するためにフラグの値を取り出す関数*/

    eflg|=EFLAGS_AC_BIT; /*ACフラグというものを1にする。これはそれぞれ異なっている要素*/
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
void memman_init(struct MEMMAN *man){
    /*初期化*/
    man->frees=0; /* あき情報の個数 */
    man->maxfrees=0; /* 状況観察用：freesの最大値 */
    man->lostsize=0; /* 解放に失敗した合計サイズ */
    man->losts=0; /* 解放に失敗した回数 */
    return ;
}

unsigned int memman_total(struct MEMMAN *man){
    /*空きサイズの合計を算出*/
    unsigned int i,t;
    for(i=0;i<man->frees;i++){
        t+=man->free[i].size;
    }
    return t;
}


unsigned int memman_alloc(struct MEMMAN *man,unsigned int size){
    unsigned int i,a;
    for(i=0;i<man->frees;i++){
        if(man->free[i].size>=size){
            /*メモリの空きが必要分あることを確認。確保をする*/
            a=man->free[i].addr; /*ここでaddrをaに格納することで格納した場所を示すことができる*/
            man->free[i].addr+=size;
            man->free[i].size-=size;
            if(man->free[i].size==0){
                /*そのリストにはメモリの空きがなくなったのでリストの個数を一つ減らす*/
                man->frees--;
                for(;i<man->frees;i++){
                    man->free[i]=man->free[i+1];/*構造体の代入*/
                }
            }
            return a;
        }
    }
    return 0;
}

int memman_free(struct MEMMAN *man,unsigned int addr,unsigned int size){
    /*空きメモリを追加する*/
    int i,j;
    /* まとめやすさを考えると、free[]がaddr順に並んでいるほうがいい */
	/* だからまず、どこに入れるべきかを決める */
    for(i=0;i<man->frees;i++){
        if(man->free[i].addr>addr){
            break;
            /*ここでiが決まる*/
        }
    }
    /* free[i - 1].addr < addr < free[i].addr */
    
    /* 前にリストに入っている空きメモリがある */
    if(i>0){
        /* 前のあき領域にまとめられる */
        if(man->free[i-1].addr+man->free[i-1].size==addr){
            man->free[i-1].size+=size;
            /* 後ろもある */
            if(i<man->frees){
                /* なんと後ろともまとめられる */
                if(addr+size==man->free[i].addr){
                    /* man->free[i]の削除 */
		    /* free[i]がなくなったので前へつめる */
                    man->free[i-1].size+=man->free[i].size;
                    man->frees--;
                    for(;i<man->frees;i++){
                        man->free[i]=man->free[i+1];/*構造体の代入*/
                    }
                }
            }
            return 0; /*成功終了*/
        }
    }

    /* 前とはまとめられなかった */

    /* 後ろに空きメモリがある */
    if(i<man->frees){
        /* 後ろとはまとめられる */
        if(addr+size==man->free[i].addr){
            man->free[i].addr=addr;
            man->free[i].size+=size;
            return 0;
        }

    }
    /* 空きメモリをリストに追加できる ここでは事前にあった空きリストには合体できなかったために空きリストに追加する*/
    if(man->frees<MEMMAN_FREES){
        /* free[i]より後ろを、後ろへずらして、後ろに追加する */
        for(j=man->frees;j>i;j--){
            man->free[j]=man->free[j-1];
        }
        man->frees++;
        if(man->maxfrees<man->frees){
            man->maxfrees = man->frees; /* 最大値を更新 */
        }
        man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* 成功終了 */
    }
    /* 後ろにずらせなかった */
	man->losts++;
	man->lostsize += size;
	return -1; /* 失敗終了 */
}

