/**FIFO ライブラリ**/
#include"bootpack.h"

#define FLAGS_OVERRUN 0x0001;

void fifo8_init(struct FIFO8 *fifo,int size ,unsigned char *buf){
    fifo->size=size;
    fifo->buf=buf;
    fifo->free=size;/*空き*/
    fifo->flags=0;
    fifo->p=0;/*書き込み位置*/
    fifo->q=0;/*読み込み位置*/
    return;
}

int fifo8_put(struct FIFO8 *fifo,unsigned char data){
    /* FIFOへデータを送り込んで蓄える */
    if(fifo->free==0){
        /*空きがなくて溢れた*/
        fifo->flags|=FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->p]=data;
    fifo->p++;
    if(fifo->p==fifo->size){
        fifo->p=0;
    }
    fifo->free--;
    return 0;
}

int fifo8_get(struct FIFO8 *fifo){
    /* FIFOからデータを一つとってくる */
    int data;
    if(fifo->free==fifo->size){
        /* バッファが空っぽのときは、とりあえず-1が返される */
		return -1;
    }
    data=fifo->buf[fifo->q];
    fifo->q++;
    if(fifo->q==fifo->size){
        fifo->q=0;
    }
    fifo->free++;
    return data;
}

int fifo8_status(struct FIFO8 *fifo)
/* どのくらいデータが溜まっているかを報告する */
{
	return fifo->size - fifo->free;
}

void fifo32_init(struct FIFO32 *fifo,int size,int *buf){
    fifo->size=size;
    fifo->buf=buf;
    fifo->free=size;
    fifo->flags=0;
    fifo->p=0;
    fifo->q=0;
    return ;
}

int fifo32_put(struct FIFO32 *fifo,int data){
    if(fifo->free==0){
        fifo->flags|=FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->p]=data;
    fifo->p++;
    if(fifo->p==fifo->size){
        fifo->p=0;
    }
    fifo->free--;
    return 0;
}

int fifo32_get(struct FIFO32 *fifo){
    int data;
    if(fifo->free==fifo->size){
        /*バッファがからの場合はとりあえず−１を返す*/
        return -1;
    }
    data=fifo->buf[fifo->q];
    fifo->q++;
    if(fifo->q==fifo->size){
        fifo->q=0;
    }
    fifo->free++;
    return data;
}

int fifo32_status(struct FIFO32 *fifo){
    return fifo->size-fifo->free;
}
