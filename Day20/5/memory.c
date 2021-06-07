/* メモリ関係 */

#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

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
            a=man->free[i].addr;
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
    /* 空きメモリをリストに追加できる*/
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

unsigned int memman_alloc_4k(struct MEMMAN *man,unsigned int size){
    unsigned int a;
    size=(size+0xfff)&0xfffff000;
    a=memman_alloc(man,size);
    return a;
}

int memman_free_4k(struct MEMMAN *man,unsigned int addr,unsigned int size){
    int i;
    size=(size+0xfff)&0xfffff000;
    i=memman_free(man,addr,size);
    return i;
}