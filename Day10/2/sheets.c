#include"bootpack.h"

/*メモリ確保と初期化をしている構造体を作成*/
struct SHTCTL *shtctl_init(struct MEMMAN *memman,unsigned char *vram,int xsize,int ysize){
    struct SHTCTL *ctl;
    int i;
    ctl=(struct SHTCTL *)memman_alloc_4k(memman,sizeof(struct SHTCTL));
    if(ctl==0){
        goto err;
    }
    ctl->vram=vram;
    ctl->xsize=xsize;
    ctl->ysize=ysize;
    ctl->top=-1;
    for(i=0;i<MAX_SHEETS;i++){
        ctl->sheets0[i].flags=0;/*未使用なのをマークする (先にsheets0から情報を書いていく,その後(ここでは行わない)sheetsに情報を書いていく)*/
    }
err:
    return ctl;
}
/*未使用の下敷きをもらってくる*/

#define SHHET_USE 1

struct SHEET *sheet_alloc(struct SHTCTL *ctl){
    struct SHEET *sht;
    int i;
    for(i=0;i<MAX_SHEETS;i++){
        if(ctl->sheets0[i].flags==0){
            sht=&ctl->sheets0[i];/*`ctl->sheets0[i]の番地 &(ctl->sheets0[i]という意味)*/
            sht->flags=SHHET_USE;/*使用中のマーク*/
            sht->height=-1;/*非表示中(ここでは設定を行っていないという理由による)*/
            return sht;
        }
    }
    return 0;/*すべてのシートが使用中*/
}
/*下敷きの高さ以外を設定している*/
void sheet_setbuf(struct SHEET *sht,unsigned char *buf,int xsize,int ysize,int col_inv){
    sht->buf=buf;
    sht->bxsize=xsize;
    sht->bysize=ysize;
    sht->col_inv=col_inv;/*透明度設定*/
    return ;
}
/*下敷きの高さを設定するための関数*/
void sheet_updown(struct SHTCTL *ctl,struct SHEET *sht,int height){
    int h;
	int old=sht->height;/* 設定前の高さを記憶する*/
    /*高さの指定が低すぎや高すぎだったら、修正を行う ここでは一番上の下敷きについて設定 topが一番上の下敷きの高さである(ここでは最初にheightは指定されたものである　ctl->topは一番上であり、ctl->top+1=heightならokだがそれよりも大きのは設定できないのでここで設定し直している)*/
    if(height>ctl->top+1){
        height=ctl->top+1;
    }
    if(height<-1){
        height=-1;
    }
    sht->height=height;/*高さを設定*/

    /*以下は主にsheets[]の並べ替え*/

    /*以前の高さよりも低くしたい。（つまりより下に行っている）*/
	//old=sht->heightである (sht->height > heightの場合) ここでは既存のsheetの高さ(height)を変える(以前の高さよりも低くする場合)
    if(old>height){
        if(height>=0){
            /* 間のものの位置を一つ引き上げる */
            for(h=old;h>height;h--){
                ctl->sheets[h]=ctl->sheets[h-1];/*ここで高さは`h-1｀になってしまっているので以下で高さを設定し直している*/
                ctl->sheets[h]->height=h;
            }
            /*ここで下敷きすべてを設定し直す*/
            ctl->sheets[height]=sht;
        }else{
            /*対象の下敷きを非表示にするための設定(ここでは、これから指定する下敷きの高さを-1にしたい場合のみである)*/
            if(ctl->top>old){
                /*上になっているものを下ろす*/
                for(h=old;h<ctl->top;h++){
                    ctl->sheets[h]=ctl->sheets[h+1];
                    ctl->sheets[h]->height=h;
                }
            }
            ctl->top--;/* 表示中の下じきが一つ減るので、一番上の高さが減る */
        }
		//対象のsheetを非表示にするそのため以下のことをやる
		//非表示にするsheetまでの描画をもう一回やり直す関数
        sheet_refresh(ctl);
    /*以前よりも高くしたい
	 *表示されているものを高くする* */
    }else if(old<height){
        if(old>=0){
            /*間のものの下敷きの位置を一つずつ下げる*/
            for(h=old;h<height;h++){
                ctl->sheets[h]= ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }else{
            /*非表示状態の下敷きを表示させる*/
            /*上のものの位置を上げる*/
            for(h=ctl->top;h>height;h--){
                ctl->sheets[h+1]=ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++; /* 表示中の下じきが一つ増えるので、一番上の高さが増える */
        }/*新しい下敷きの情報に沿って画面を描き直す*/
   		sheet_refresh(ctl);
    }
    return ;
}
/*ここでは描画し直す*/
void sheet_refresh(struct SHTCTL *ctl){
    int h,bx,by,vx,vy;
    unsigned char *buf,c,*vram=ctl->vram;
    struct SHEET *sht;
    for(h=0;h<=ctl->top;h++){
        sht=ctl->sheets[h];
        buf=sht->buf;
        for(by=0;by<sht->bysize;by++){
            /*vy0,vx0は下敷きの画面上での位置。bxsize,bysizeは下敷きの大きさ*/
            vy=sht->vy0+by;
            for(bx=0;bx<sht->bxsize;bx++){
                vx=sht->vx0+bx;
                c=buf[by*sht->bxsize+bx];
                if(c!=sht->col_inv){
                    vram[vy*ctl->xsize+vx]=c;
                }
            }
        }
    }
    return ;
}
/*下敷きを動かす(2次元)*/
void sheet_slide(struct SHTCTL *ctl,struct SHEET *sht,int vx0,int vy0){
    sht->vx0=vx0;
    sht->vy0=vy0;
    if(sht->height>=0){
        sheet_refresh(ctl);
    }
    return ;
}
/*下敷きを解放する*/
void sheet_free(struct SHTCTL *ctl,struct SHEET *sht){
    if(sht->height>=0){
        sheet_updown(ctl,sht,-1);
    }
    sht->flags=0;
    return ;
}