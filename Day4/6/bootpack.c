/*naskfunc.asmで作った関数*/

/*アセンブリで作った関数を使うために最初に宣言する*/
extern void _io_hlt(void);
extern void _io_cli(void);
extern void _io_out8(int port,int data);
extern int _io_load_eflags(void);
extern void _io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start,int end,unsigned char *rgb);


void HariMain(void){
    int i; /*変数宣言*/
    char *p; /*pという変数は、BYTE用の番地を示す*/

    init_palette();

    p=(char*)0xa0000;

    for (i=0;i<0xffff;i++){
        p[i]=i&0x0f;
    }


    for(;;){
        _io_hlt();
    }
}

void init_palette(void){
    static unsigned char table_rgb[16*3]={
        0x00, 0x00, 0x00, /*黒*/
        0xff, 0x00, 0x00, /*明るい赤*/
        0x00, 0xff, 0x00, /*明るい緑*/
        0xff, 0xff, 0x00, /*明るい黄*/
        0x00, 0x00, 0xff, /*明るい青*/
        0xff, 0x00, 0xff, /*明るい紫*/
        0x00, 0xff, 0xff, /*明るい水色*/
        0xff, 0xff, 0xff, /*白*/
        0xc6, 0xc6, 0xc6, /*明るい灰色*/
        0x84, 0x00, 0x00, /*暗い赤*/
        0x00, 0x84, 0x00, /*暗い緑*/
        0x84, 0x84, 0x00, /*暗い黄*/
        0x00, 0x00, 0x84, /*暗い青*/
        0x84, 0x00, 0x84, /*暗い紫*/
        0x00, 0x84, 0x84, /*暗い水色*/
        0x84, 0x84, 0x84, /*暗い灰色*/
        //ここで8bitの場合は、色番号を自分で設定することができるために必要な色を自分で決めることができる
    };
    set_palette(0,15,table_rgb);
    return ;
}

void set_palette(int start, int end ,unsigned char *rgb){
    int i,eflags;
    eflags=_io_load_eflags();    /*割り込み許可を数えるフラグ*/
    _io_cli();                   /*フラグを0にする　ここで割り込み禁止にしている*/
    _io_out8(0x03c8,start);		/*0x03c8にデータを書き込むとパレットを書き込むモードになる*/
    for(i=start;i<=end;i++){
        _io_out8(0x03c9,rgb[0]/4);/*0x03c9にデータを書き込む順番はRGB*/
        _io_out8(0x03c9,rgb[1]/4);
        _io_out8(0x03c9,rgb[2]/4);
        rgb+=3;
    }
    _io_store_eflags(eflags);/*フラグを１にして割り込み禁止を解除する*/
    return ;
}
