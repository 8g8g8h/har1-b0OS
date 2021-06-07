int api_openwin(char *buf,int xsiz,int ysiz,int col_inv,char *title);
void api_bookfillwin(int win,int x0,int y0,int x1,int y1,int col);
void api_initmalloc(void);
char *api_malloc(int size);
void api_point(int win,int x,int y,int col);
void api_end(void);

unsigned long rand(void);


unsigned long rand(void){
  static unsigned long rand;

  rand*=1234567;
  rand+=1332;

  return rand;
}

void HariMain(void){
  char *buf;
  int win,i,x,y;
  api_initmalloc();
  buf=api_malloc(150*100);
  win=api_openwin(buf,150,100,-1,"stars");
  api_bookfillwin(win,6,26,143,93,0/*黒*/);
  for(i=0;i<50;i++){
    x=(rand()%137)+6;
    y=(rand()%67)+26;
    api_point(win,x,y,3/*黄色*/);
  }
  api_end();
}
