#include"bootpack.h"

void init_gdtidt(void){
    struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    struct GATE_DESCRIPTOR *idt=(struct GATE_DESCRIPTOR *) ADR_IDT;
    int i;

    /*GDTの初期化*/
    for(i=0;i<=LIMIT_GDT/8;i++){
        set_segmdesc(gdt+i,0,0,0);
    }
    set_segmdesc(gdt+1,0xffffffff,0x00000000,AR_DATA32_RW);
    set_segmdesc(gdt+2,LIMIT_BOTPAK,ADR_BOTPAK,AR_CODE32_ER);
    load_gdtr(LIMIT_GDT,ADR_GDT);

    /*IDTの初期化*/
    for(i=0;i<=LIMIT_IDT/8;i++){
         set_gatedesc(idt+i,0,0,0);
    }
    load_idtr(LIMIT_IDT,ADR_IDT);

    /*IDTの設定*/
    set_gatedesc(idt+0x20,(int) asm_inthandler20,2*8,AR_INTGATE32);
    set_gatedesc(idt+0x21,(int) asm_inthandler21,2*8, AR_INTGATE32);
    set_gatedesc(idt+0x27, (int) asm_inthandler27, 2*8,AR_INTGATE32);
    set_gatedesc(idt+0x2c, (int) asm_inthandler2c,2*8,AR_INTGATE32);

    return ;

}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd,unsigned int limit,int base,int ar){
    if(limit>0xfffff){
        ar|=0x8000;/*G_bit=1にする。これによりリミットをページ単位にする*/
        limit/=0x1000;
    }
    sd->limit_low=limit&0xffff;
    sd->base_low=base&0xffff;
    sd->base_mid=(base>>16)&0xff;
    sd->access_right=ar&0xff;
    sd->limit_high=((limit>>16)&0x0f)|((ar>>8)&0xf0);
    sd->base_high=(base>>24)&0xff;
    return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd,int offset,int selector,int ar){
    gd->offset_low=offset&0xffff;
    gd->selector=selector;
    gd->dw_count=(ar>>8)&0xff;
    gd->access_right=ar&0xff;
    gd->offset_high=(offset>>16)&0xffff;
    return;
}

/*
struct SEGMENT_DESCRIPTOR{
    //short 2byte char 1byte
    /baseはセグメントの番地について
    それぞれ分けて格納
    全部で32bitで表現する
    low(2byte),mid(1byte),high(1byte)
    これでセグメントのベース番地を表現する

    ここでも大きさは4GBつまり32bitの数値になるが構造体に他のデータが入らなくなる
    ここではlimitは20bitがセグメントの大きさを示す
    しかしこれでは全部表現できない
    最大で1MBしか表現できない(2^20=1MB)
    その際にGbitのフラグが1ならばリミットをバイト単位でなく、ページ単位にする
    ここでページ単位は4KBである
    よって4KB*1MB=4GB
    
    残りの12bitでアクセス権限について設定する

    short limit_low,base_low;
    char base_mid,access_right;
    char limit_high,base_high;
};

struct GATE_DESCRIPTOR{
    セレクタ(16bitで、selectorに格納)
    保持している割り込みハンドラの存在するセグメント番号。
    
    オフセット(32bitで、上位16bitはoffset_high、下位16bitはoffset_lowに格納)
    保持している割り込みハンドラの、「セレクタが示すセグメント」の先頭からのアドレス。
    
    種類(8bitで、typeに格納)
    とりあえず0x8E。あまり深く考えなくてよい。(特権レベル0で32bitの割り込みゲート)
    
    countはコピーカウントを表すそうですが、とりあえず現段階ではあまり考える必要はないとのこと。
    
    short offset_low,selector;
    char dw_count,access_right;
    short offset_high;
};


IDTとは先程のGATE_DESCRIPTORの構造体を配列として格納したもの
よって構造体の数を必要な量作ることでテーブルが完成する

割り込みの仕組み
割り込み通知が来た際にIDTの場所を知らせることが必要
そのためにIDTRレジスタに格納する
これ(IDTR)は、48bitのシステムレジスタで、
上位16bitがIDTのリミット値であり、IDTに格納されているディスクリプタの数-1で、
下位32bitがIDTの先頭アドレスです。

これで、CPUは割り込みが発生したときにIDTRからアドレスを読みだしてIDTを読み込みます。
IDTを読み込んだ後、
割り込み番号に対応するディスクリプタ(構造体)を読みだします。
そのディスクリプタのオフセットをeipレジスタ、セレクタをcs（セグメントレジスタの1つ）レジスタにセットします。
[cs:eip]により割り込みの設定が入っているメモリを特定することができる


*/
