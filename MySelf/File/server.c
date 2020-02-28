#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include "tools.h"

#define pf printf

int clifd = 0;
char cmd[20] = {};

void* start_run(void *arg);
void c_up(void);
void c_down(void);
void c_list(void);

typedef struct LS
{
  char mode[15];  // 文件的模式
  int dir_num;  // 是否目录或目录中包含目录的数量
  char user[20];  // 文件的用户名
  char group[20]; // 文件的组名
  long size;    // 文件的字节数
  char time[30];  // 文件的最后修改时间
  int st_mode;  // 文件类型和权限
  char name[20];  // 文件名
}LS; 

int main()
{
  pf("服务器创建socket...\n");
  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(0 > sockfd)
  {
    perror("socket");
    return -1;
  }

  pf("准备地址...\n");
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9090);
  addr.sin_addr.s_addr = inet_addr("0.0.0.0");
  socklen_t len = sizeof(addr);

  pf("绑定socket与地址...\n");
  if(bind(sockfd,(struct sockaddr*)&addr,len))
  {
    perror("bind");
    return -1;
  }

  pf("设置监听...\n");
  if(listen(sockfd,5))
  {
    perror("listen");
    return -1;
  }

  pf("等待客户端连接...\n");
  for(;;)
  {
    struct sockaddr_in addrcli = {};
    clifd = accept(sockfd,(struct sockaddr*)&addrcli,&len);
    if(0 > clifd)
    {
      perror("accept");
      continue;
    }
    pthread_t pid;    
    pthread_create(&pid,NULL,start_run,&clifd);
  }
}

void* start_run(void *arg)
{
  (void)arg;
  for(;;)
  { 
    int c_size = read(clifd,cmd,sizeof(cmd));
    char up[20] = "我想上你";
    char down[20] = "我想下你";
    char see[20] = "我想看你";
    if(strcmp(up,cmd)==0)
    {
      pf("收到客户端的上传指令\n");
      c_up();
      memset(&cmd,0,20);
    }
    if(strcmp(down,cmd)==0)
    {
      pf("收到客户端的下载指令\n");
      c_down();
      memset(&cmd,0,20);
    }
    if(strcmp(see,cmd)==0)
    {
      pf("收到客户端的目录指令\n");
      c_list();
      memset(&cmd,0,20);
    }
  }
  //char *str = "我死了"; 
  //  //pthread_exit(str);
  //
}

void c_up(void)
{
  char filename[50] = {};
  int f_size = read(clifd,filename,sizeof(filename));
  pf("filename:%s\n",filename);
  int fd = open(filename,O_CREAT|O_RDWR,0777);
  pf("buf...\n");

  int flag = 0; 
  int r_size = 0;
  int w_size = 0;
  do{
    char buf[1024] = {};
    r_size = read(clifd,buf,sizeof(buf));
    pf("r_size:%d\n",r_size);
    w_size = write(fd,buf,r_size);
    pf("w_size:%d\n",w_size);
    flag++;
  }while(r_size==1024);

  sleep(1);

  if(flag > 0)
  {
    char result[20] = "success";
    pf("%s\n",result);
    write(clifd,result,strlen(result)+1);
  }
  else
  {
    char result[20] = "error";
    pf("%s\n",result);
    write(clifd,result,strlen(result)+1);
  } 
  close(fd);      
  return; 
}

void c_down(void)
{
  DIR *dir;
  dir = opendir(".");
  char list[1024] = {};
  char *dou = ",";  
  struct dirent *dirent;
  while((dirent = readdir(dir)) != NULL)
  {
    strcat(list,dirent->d_name);
    strcat(list,dou);
  }

  int l_size = write(clifd,list,strlen(list)+1);

  char filename[50] = {};
  int f_size = read(clifd,filename,sizeof(filename));
  pf("filename:%s\n",filename);
  if(strstr(filename,",")!= NULL || strstr(list,filename) == NULL)
  {
    char result[20] = "文件不存在";
    pf("%s\n",result);
    write(clifd,result,strlen(result)+1);
  }
  else
  {
    char result[20] = "文件存在";
    pf("%s\n",result);
    write(clifd,result,strlen(result)+1);

    int fd = open(filename, O_RDONLY);

    sleep(1);

    int r_size = 0;
    int w_size = 0;
    do{
      char buf[1024] = {};
      r_size = read(fd,buf,sizeof(buf));
      pf("r_size:%d\n",r_size);
      w_size = write(clifd,buf,r_size);
      pf("w_size:%d\n",w_size);
      usleep(500);
    }while(r_size == 1024);
    sleep(1);   
    pf("发送完毕\n");
    close(fd);
  }
  return;
}

void c_list(void)
{
  DIR *dir;
  dir = opendir(".");
  char list[1024] = {};
  char *dou = ",";  
  struct dirent *dirent;
  while((dirent = readdir(dir)) != NULL)
  {
    strcat(list,dirent->d_name);
    strcat(list,dou);
  }

  int l_size = write(clifd,list,strlen(list)+1);

  memset(list,0,1024);

  char dirname[20] = {};  
  int d_size = read(clifd,dirname,sizeof(dirname));
  if(strcmp(dirname,".") == 0)
  {
    dir = opendir(".");
    while((dirent = readdir(dir)) != NULL)
    {
      strcat(list,dirent->d_name);
      strcat(list,dou);
    }
    l_size = write(clifd,list,strlen(list)+1);

  }
  if(strcmp(dirname,"..") == 0)
  {
    chdir("..");
    dir = opendir(".");
    while((dirent = readdir(dir)) != NULL)
    {
      strcat(list,dirent->d_name);
      strcat(list,dou);
    }
    l_size = write(clifd,list,strlen(list)+1);
  }
  else
  {
    int re = chdir(dirname);
    if(re == -1)
    {
      char result[20] = "目录名错误";
      int err = write(clifd,result,strlen(result)+1);
    }
    else
    {
      dir = opendir(".");
      while((dirent = readdir(dir)) != NULL)
      {
        strcat(list,dirent->d_name);
        strcat(list,dou);
      }
      l_size = write(clifd,list,strlen(list)+1);
    }
  }
  closedir(dir);
  return;
}
