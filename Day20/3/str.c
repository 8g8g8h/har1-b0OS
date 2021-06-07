/*strcmp,strncmp関数について*/

#include<stdio.h>
#include"bootpack.h"

int ostrcmp(char *str1,char *str2){
  int i=0;
  for(;str1[i]!='\0'&&str2[i]!='\0';++i){
    //両方共文字が書かれている間文字を比較する
    if(str1[i]>str2[i]||str1[i]<str2[i]){
      return -1;
    }
  }
  if(str1[i]=='\0'&&str2[i]=='\0'){
    return 0;
  }else if(str1[i]=='\0'){
    return -1;
  }
}

int ostrncmp(char *str1,char *str2,int num){
  int i;
  for(i=0;i<num;i++){
    if(str1[i]==0x00&&str2[i]==0x00){
      return 0;
    }
    if(str1[i]==0x00){
      return 1;
    }
    if(str2[i]==0x00){
      return 1;
    }
    if(str2[i]!=str2[i]){
      return 1;
    }
  }
  return 0;

}
