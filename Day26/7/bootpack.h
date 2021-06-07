/*asmhead.asm*/
struct BOOTINFO{
    char cyls;/*0x0ff0-0x0fff*/
    char leds;/*ブートセクタはどこまでディスクを読んだのか*/
    char vmode;/*ブート時のキーボードのLED状態*/
    char reserve;/*ビデオモード　何ビットカラーか*/
    short scrnx,scrny; /*画像解像度*/
    char *vram;
};

#define ADR_BOOTINFO 0x00000ff0
#define ADR_DISKIMG  0x00100000

/*naskfunc.asm*/
void _io_hlt(void);
void _io_cli(void);
void _io_sti(void);
void _io_stihlt(void);
int _io_in8(int port);
void _io_out8(int port,int data);
int _io_load_eflags(void);
void _io_store_eflags(int eflags);
void load_gdtr(int limit,int addr);
void load_idtr(int limit,int addr);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void asm_inthandler0d(void);
void asm_inthandler0c(void);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
void farjmp(int eip,int cs);
void farcall(int eip,int cs);
void asm_hrb_api(void);
void start_app(int eip,int cs,int esp,int ds,int *tss_esp0);
void asm_end_app(void);

/*a_nasm.asm*/
int api_openwin(char *buf,int xsiz,int ysiz,int col_inv,char *title);
void api_putstrwin(int win,int x,int y,int col,int len,char *str);
void api_bookfillwin(int win,int x0,int y0,int x1,int y1,int col);
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr,int size);
void api_point(int win,int x,int y,int col);
void api_end(void);
void api_refreshwin(int win,int x0,int y0,int x1,int y1);
void api_linewin(int win,int x0,int y0,int x1,int y1,int col);
void api_closewin(int win);
int api_getkey(int mode);
void api_initmalloc(void);
int api_alloctimer(void);
void api_inittimer(int timer, int data);
void api_settimer(int timer, int time);
void api_beep(int tone);

/*fifo.c*/
struct FIFO8{
    unsigned char *buf;/* FIFOバッファ構造体の場所を格納する変数*/
    int p,q,size,free,flags;/*p= 書き込み、q＝読み込み、free= バッファの空き容量、size=構造体の総容量、flags=溢れたかの確認するためのフラグ*/
} ;
struct FIFO32{
   int *buf;
   int p,q,size,free,flags;
   struct TASK *task;
};
void fifo8_init(struct FIFO8 * fifo,int size,unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo,unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

void fifo32_init(struct FIFO32 *fifo,int size,int *buf,struct TASK *task);
int fifo32_put(struct FIFO32 *fifo,int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

/*graphics.c*/
void _init_palette(void);
void _set_palette(int start,int end,unsigned char *rgb);
void bookfill8(unsigned char *vram,int xsize,unsigned char c,int x0,int y0,int x1,int y1);
void init_screen(char *vram, int x, int y);
void putfont8(char *vram,int xsize,int x,int y,char c,char *font);
void putfont8_asc(char *vram, int xsize,int x,int y,char c, unsigned char *s);
void init_mouse_cursor8(char *mouse,char bc);
void putblock8_8(char *vram,int vxsize,int pxsize,int pysize,int px0,int py0,char *buf,int bxsize);

#define COL8_000000	0
#define COL8_FF0000	1
#define COL8_00FF00	2
#define COL8_FFFF00	3
#define COL8_0000FF	4
#define COL8_FF00FF	5
#define COL8_00FFFF	6
#define COL8_FFFFFF	7
#define COL8_C6C6C6	8
#define COL8_840000	9
#define COL8_008400	10
#define COL8_848400	11
#define COL8_000084	12
#define COL8_840084	13
#define COL8_008484	14
#define COL8_848484	15

/*dsctbl.c*/ 
struct SEGMENT_DESCRIPTOR{
    /*short 2byte char 1byte*/
    short limit_low,base_low;
    char base_mid,access_right;
    char limit_high,base_high;
};

struct GATE_DESCRIPTOR{
    short offset_low,selector;
    char dw_count,access_right;
    short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd,unsigned int limit,int base,int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd,int offset,int selector,int ar);

#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff
#define ADR_BOTPAK 0x00280000
#define LIMIT_BOTPAK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define AR_INTGATE32 0x008e
#define AR_TSS32		0x0089

/*int.c*/
struct KEYBUF {
	unsigned char data[32];
    int next_w,next_r,len;
};
void init_pic(void);
void inthandler27(int *esp);


#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

/* keyboard.c */
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo,int data0);
extern struct FIFO32 * keyfifo;
#define PORT_KEYDAT		0x0060
#define PORT_KEYCMD		0x0064

/* mouse.c */
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void inthandler2c(int *esp);
void enable_mouse(struct FIFO32 *fifo,int data0,struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
extern struct FIFO32 *mousefifo;

/* memory.c */
#define MEMMAN_FREES		4090	/* これで約32KB */
#define MEMMAN_ADDR			0x003c0000
struct FREEINFO {	/* あき情報 */
	unsigned int addr, size;
};
struct MEMMAN {		/* メモリ管理 */
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/*sheet.c*/
#define MAX_SHEETS 256
/*下敷きの構造体*/
struct SHEET{
    unsigned char *buf;/*この下敷きの情報が格納されている番地*/
    int bxsize,bysize,vx0,vy0,col_inv,height,flags;/*bxsize*bysize=下敷き全体の大きさ。vx0,vy0は下敷きの位置。flagsは設定情報。heightは下敷きの高さ。これは下から何枚目かを数える変数*/
	struct SHTCTL *ctl;
  struct TASK *task;
};
/*下敷きの情報を管理する構造体*/
struct SHTCTL{
    unsigned char *vram,*map; /*BOOTINFOに確認に行くのが大変なのでここに書いておくこととする*/
    int xsize,ysize,top;/*top=下敷きの高さ。これはZ軸としてみなす。一番最初に見える画面が一番下にある下敷きからどのくらいの高さがあるかを示す変数*/
    struct SHEET *sheets[MAX_SHEETS]; /*これは下のsheetsの情報を高さの低い順番に格納するためのもの　その際にメモリで管理する*/
    struct SHEET sheets0[MAX_SHEETS];/*これzは1枚の下敷きの情報（番地）を格納する配列(こちらは順番関係ない)*/
};

struct SHTCTL *shtctl_init(struct MEMMAN *memman,unsigned char *vram,int xsize,int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht,unsigned char *buf,int xsize,int ysize,int clo_inv);
//void sheet_refreshsub(struct SHTCTL *cttl,int vx0,int vy0,int vx1,int vy1);
void sheet_updown(struct SHEET *sht,int height);
void sheet_refresh(struct SHEET *sht,int bx0,int by0,int bx1,int by1);
void sheet_slide(struct SHEET *sht,int vx0,int vy0);
void sheet_free(struct SHEET *sht);

/*timer.c*/

#define MAX_TIMER	500

struct TIMER{
    //nextは次のタイマを示している 
    struct TIMER *next;
	unsigned int flags,flags2;//flags2は、アプリが終了したときに自動でキャンセルするタイマかどうかを判断する
	unsigned int timeout; //予定時刻という意味にする
	struct FIFO32 *fifo;
	int data;
};
struct TIMERCTL{
	unsigned int count,next;//nextは次のタイマのタイムアウト時間を格納する変数
	//ここではタイマの順番を入れ替えるのが目的であるためポインタ型であるが(timers)、timer0はそのものを作らなければならないのでそのままである。
	struct TIMER *t0; //最初のタイマだけを格納すれば良くなったためひとつ
	struct TIMER timers0[MAX_TIMER];
};
extern struct TIMERCTL timerctl;
void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer,struct FIFO32 *fifo,int data);
void timer_settime(struct TIMER *timer,unsigned int timeout);
void inthandler20(int *esp);
int timer_cancel(struct TIMER *timer);
void timer_cancelall(struct FIFO32 *fifo);


/**mtask.c**/
#define MAX_TASKS 1000  /*最大タスク*/
#define TASK_GDT0 3     /*TSSをGDTの何番から割り当てるのか*/
#define MAX_TASKS_LV  100
#define MAX_TASKLEVELS  10

//タスクスイッチするために設定するレジスタなどを集めたもの
struct TSS32{
  int backlink,esp0,ss0,esp1,ss1,esp2,ss2,cr3;
  int eip,eflags,eax,ecx,edx,ebx,esp,ebp,esi,edi;
  int es,cs,ss,ds,fs,gs;
  int ldtr,iomap;
};

//1つずつのタスクについて
struct TASK{
  int sel,flags;/*selはGDTの番号*/
  int level,priority;
  struct TSS32 tss;
  struct FIFO32 fifo;
  struct CONSOLE *cons;
  int ds_base;
  int cons_stack;
};

struct TASKLEVEL{
  int running;/*動作しているタスク数*/
  int now;/*現在動作しているタスクを識別するための変数*/
  struct TASK *tasks[MAX_TASKS_LV];
};

struct TASKCTL{
  int now_lv;/*現在動作中のレベル*/
  char lv_change;/*次のタスクスイッチのときに、レベルを変えるかどうかを書き込む変数*/
  struct TASKLEVEL level[MAX_TASKLEVELS];
  struct TASK tasks0[MAX_TASKS];
};
extern struct TIMER *task_timer;
struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task,int level,int priority);
void task_switch(void);
void task_sleep(struct TASK *task);
struct TASK *task_now(void);
void task_add(struct TASK *task);
void task_remove(struct TASK *task);
void task_switchsub(void);
void task_idle(void);

#define KEYCMD_LED 0xed


/*window.c*/
void make_window8(unsigned char *buf,int xsize,int ysize,char *title,char act);
void make_wtitle8(unsigned char *buf,int xsize,char *title,char act);
void putfonts8_asc_sht(struct SHEET *sht,int x,int y,int c,int b,char *s,int l);
void make_textbook8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void change_wtitle8(struct SHEET *sht,char act);


/*file.c*/

struct FILEINFO{
  unsigned char name[8],ext[3],type;
  char reserve[10];
  unsigned short time,date,clustno;
  unsigned int size;
};

void file_readfat(int *fat,unsigned char *img);

void file_loadfile(int clustno,int size,char *buf,int *fat,char *img);

struct FILEINFO *file_search(char *cmdline,struct FILEINFO *finfo,int max);

/*console.c*/

struct CONSOLE{
  struct SHEET *sht;
  int cur_x,cur_y,cur_c;
  struct TIMER *timer;
};

void console_task(struct SHEET *sheet,int memtotal);

void cons_putchar(struct CONSOLE *cons,int chr,char move);

void cons_newline(struct CONSOLE *cons);

void cons_runcmd(char *cmdline,struct CONSOLE *cons,int *fat,int memtotal);

void cmd_mem(struct CONSOLE *cons,int memtotal);

void cmd_clear(struct CONSOLE *cons);

void cmd_ls(struct CONSOLE *cons);

void cmd_exit(struct CONSOLE *cons,int *fat);

void cmd_cat(struct CONSOLE *cons,int *fat,char *cmdline);

int cmd_app(struct CONSOLE *cons,int *fat,char *cmdline);

void cons_putstr0(struct CONSOLE *cons,char *s);

void cons_putstr1(struct CONSOLE *cons,char *s,int l);

int *hrb_api(int edi,int esi,int ebp,int esp,int ebx,int edx,int ecx,int eax);

int *inthandler0d(int *esp);

int *inthandler0c(int *esp);

void hrb_api_linewin(struct SHEET *sht,int x0,int y0,int x1,int y1,int col);
/*str.c*/

int ostrcmp(char *str1,char *str2);

int ostrncmp(char *str1,char *str2,int num);

