#include"bootpack.h"

/*メモリ確保と初期化をしている構造体を作成*/
struct SHTCTL *shtctl_init(struct MEMMAN *memman,unsigned char *vram,int xsize,int ysize){
    struct SHTCTL *ctl;
    int i;
    ctl=(struct SHTCTL *)memman_alloc_4k(memman,sizeof(struct SHTCTL));
    if(ctl==0){
        goto err;
    }
	ctl->map=(unsigned char *)memman_alloc_4k(memman,xsize*ysize);
	if(ctl->map==0){
		memman_free_4k(memman,(int)ctl,sizeof(struct SHTCTL));
		goto err;
	}
    //描画するために必要な情報なのでここで取得することにする(vram,xsize,ysize)
    ctl->vram=vram;
    ctl->xsize=xsize;
    ctl->ysize=ysize;
    ctl->top=-1;
    for(i=0;i<MAX_SHEETS;i++){
        ctl->sheets0[i].flags=0;/*未使用なのをマークする (先にsheets0から情報を書いていく,その後(ここでは行わない)sheetsに情報を書いていく)*/
		ctl->sheets0[i].ctl=ctl;
	}
err:
    return ctl;
}
/*未使用の下敷きをもらってくる*/

#define SHEET_USE 1

struct SHEET *sheet_alloc(struct SHTCTL *ctl){
    struct SHEET *sht;
    int i;
    for(i=0;i<MAX_SHEETS;i++){
        if(ctl->sheets0[i].flags==0){
            sht=&ctl->sheets0[i];/*`ctl->sheets0[i]の番地*/
            sht->flags=SHEET_USE;/*使用中のマーク*/
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
void sheet_refreshmap(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0){
	int h,bx,by,vx,vy,bx0,by0,bx1,by1;
	unsigned char *buf ,sid,*map=ctl->map;
	struct SHEET *sht;
	if(vx0<0){
		vx0=0;
	}
	if(vy0<0){
		vy0=0;
	}
	if(vx1>ctl->xsize){
		vx1=ctl->xsize;
	}
	if(vy1>ctl->ysize){
		vy1=ctl->ysize;
	}
	for(h=h0;h<=ctl->top;h++){
		sht=ctl->sheets[h];
		sid=sht-ctl->sheets0;
		buf=sht->buf;
		bx0=vx0-sht->vx0;
		by0=vy0-sht->vy0;
		bx1=vx1-sht->vx0;
		by1=vy1-sht->vy0;
		if(bx0<0){
			bx0=0;
		}
		if(by0<0){
			by0=0;
		}
		if(bx1>sht->bxsize){
			bx1=sht->bxsize;
		}
		if(by1>sht->bysize){
			by1=sht->bysize;
		}
		for(by=by0;by<by1;by++){
			vy=sht->vy0+by;
			for(bx=bx0;bx<bx1;bx++){
				vx=sht->vx0+bx;
				if(buf[by*sht->bxsize+bx]!=sht->col_inv){
					map[vy*ctl->xsize+vx]=sid;
				}
			}
		}
	}
	return ;
}
void sheet_refreshsub(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0,int h1){
	int h,bx,by,vx,vy,bx0,by0,bx1,by1;
	unsigned char *buf,c,*vram=ctl->vram,*map=ctl->map,sid;
	struct SHEET *sht;
	/*refresh範囲が画面外にはみ出していたら補正*/
	if(vx0<0){
		vx0=0;
	}
	if(vy0<0){
		vy0=0;
	}
	if(vx1>ctl->xsize){
		vx1=ctl->xsize;
	}
	if(vy1>ctl->ysize){
		vy1=ctl->ysize;
	}
	for(h=h0;h<=h1;h++){
		sht=ctl->sheets[h];
		buf=sht->buf;
		sid=sht-ctl->sheets0;
		/*vx0~vy1を使って、bx0~by1を逆算する
		 * vx0=sht->vx0+bx0;
		 * vx1=sht->vx0+bx1;
		 * */
		bx0=vx0-sht->vx0;/*画面座標からシートの座標に変換*/
		by0=vy0-sht->vy0;
		bx1=vx1-sht->vx0;
		by1=vy1-sht->vy0;
		if(bx0<0){
			bx0=0;
		}
		if(by0<0){
			by0=0;
		}
		if(bx1>sht->bxsize){
			bx1=sht->bxsize;

		}
		if(by1>sht->bysize){
			by1=sht->bysize;
		}
		for(by=by0;by<by1;by++){
			vy=sht->vy0+by;
			for(bx=bx0;bx<bx1;bx++){
				vx=sht->vx0+bx;
				if(map[vy*ctl->xsize+vx]==sid){
						vram[vy*ctl->xsize+vx]=buf[by*sht->bxsize+bx];
					}
				}
			}
		}
	return ;
}
/*下敷きの高さを設定するための関数*/
void sheet_updown(struct SHEET *sht,int height){
    struct SHTCTL *ctl=sht->ctl;
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
			sheet_refreshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height+1);
			sheet_refreshsub(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height+1,old);
        }else{
            /*対象の下敷きを非表示にするための設定*/
            if(ctl->top>old){
                /*上になっているものを下ろす*/
                for(h=old;h<ctl->top;h++){
                    ctl->sheets[h]=ctl->sheets[h+1];
                    ctl->sheets[h]->height=h;
                }
            }
            ctl->top--;/* 表示中の下じきが一つ減るので、一番上の高さが減る */
			sheet_refreshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,0);
			sheet_refreshsub(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,0,old-1);
        }
	//対象のsheetを非表示にするそのため以下のことをやる
	//非表示にするsheetまでの描画をもう一回やり直す関数
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
            for(h=ctl->top;h>=height;h--){
                ctl->sheets[h+1]=ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++; /* 表示中の下じきが一つ増えるので、一番上の高さが増える */
        }/*新しい下敷きの情報に沿って画面を描き直す*/
		sheet_refreshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height);
   		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,height,height);
    }
    return ;
}
/*ここでは描画し直す*/
void sheet_refresh(struct SHEET *sht,int bx0,int by0,int bx1,int by1){
		if(sht->height>=0){
			sheet_refreshsub(sht->ctl,sht->vx0+bx0,sht->vy0+by0,sht->vx0+bx1,sht->vy0+by1,sht->height,sht->height);
		}
		return;
}
/*下敷きを動かす(2次元)*/
void sheet_slide(struct SHEET *sht,int vx0,int vy0){
	struct SHTCTL *ctl=sht->ctl;
	int old_vx0=sht->vx0,old_vy0=sht->vy0;
    sht->vx0=vx0;
    sht->vy0=vy0;
    if(sht->height>=0){
        sheet_refreshmap(ctl,old_vx0,old_vy0,old_vx0+sht->bxsize,old_vy0+sht->bysize,0);
		sheet_refreshmap(ctl,vx0,vy0,vx0+sht->bxsize,vy0+sht->bysize,sht->height);
		sheet_refreshsub(ctl,old_vx0,old_vy0,old_vx0+sht->bxsize,old_vy0+sht->bysize,0,sht->height-1);
		sheet_refreshsub(ctl,vx0,vy0,vx0+sht->bxsize,vy0+sht->bysize,sht->height,sht->height);
    }
    return ;
}
/*下敷きを解放する*/
void sheet_free(struct SHEET *sht){
    if(sht->height>=0){
        sheet_updown(sht,-1);
    }
    sht->flags=0;
    return ;
}
