int api_openwin(char *buf,int xsiz,int ysiz,int col_inv,char *title);
void api_bookfillwin(int win,int x0,int y0,int x1,int y1,int col);
void api_initmalloc(void);
char *api_malloc(int size);
void api_point(int win,int x,int y,int col);
void api_end(void);

void HariMain(void){
  char *buf;
  int win;
  api_initmalloc();
  buf=api_malloc(150*50);/*描画するために、メモリを確保しないといけない*/
  win=api_openwin(buf,150,100,-1,"star1");
  api_bookfillwin(win,6,26,143,93,0/*黒*/);
  api_point(win,75,59,3/*黄色*/);
  api_end();
}
