#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include "tools.h"

#define pf printf

int sockfd = 0;

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

void menu(void);
void upload(void);
void download(void);
void list(void);

int main(int argc,char* argv[1])
{
  if(argc < 2)
  {
    pf("./client ip");
  }

  pf("服务器创建socket...\n");
  sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(0 > sockfd)
  {
    perror("socket");
    return -1;

  }

  pf("准备地址...\n");
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9090);
  addr.sin_addr.s_addr = inet_addr(argv[1]);
  socklen_t len = sizeof(addr);

  pf("绑定连接服务器...\n");
  if(connect(sockfd,(struct sockaddr*)&addr,len))
  {
    perror("connect");
    return -1;

  }

  menu(); // 加载菜单

  close(sockfd);

}

void menu(void)
{
  for(;;)
  { 
    system("clear");
    pf("*** 局域网文件传输客户端 ***\n");
    pf("     1、上传\n");
    pf("     2、下载\n");
    pf("     3、查看/修改服务端目录\n");
    pf("     0、退出\n");
    pf("--------------------------\n");
    switch(get_cmd('0','3'))
    {
      case '1': upload(); break;
      case '2': download(); break;
      case '3': list(); break;
      case '0': return;

    }
  }


}

void upload(void)
{
  char up[20] = "我想上你";
  write(sockfd,up,strlen(up)+1);

  pf("请输入文件名:");
  char pathname[100] = {};
  char *filename = (char*)malloc(50);
  get_str(pathname,100);
  int fd = open(pathname, O_RDONLY);

  struct stat stat = {};
  int fs = fstat(fd, &stat);
  long file_size = 0; 
  file_size = stat.st_size;

  if(fd == -1)
  {
    pf("文件不存在\n");
    getchar();

  }
  else
  {
    if(strrchr(pathname,'/') == NULL)
    {
      strcpy(filename,pathname);

    }
    else
    {
      filename = strrchr(pathname,'/');
      filename += 1;

    }

    write(sockfd,filename,strlen(filename)+1);

    int r_size = 0;
    int w_size = 0;
    sleep(1);
    do{
      char buf[1024] = {};
      r_size = read(fd,buf,1024);
      pf("r_size:%d\n",r_size);
      w_size = write(sockfd,buf,r_size);
      printf("w_size:%d\n",w_size);
      usleep(500);

    }while(r_size==1024);

    sleep(1);

    char result[20] = {};
    pf("result:");
    read(sockfd,result,sizeof(result));
    pf("%s\n",result);
    getchar();

  }

  //close(fd);
  return;

}

void download(void)
{
  char down[20] = "我想下你";
  write(sockfd,down,strlen(down)+1);

  char list[1024] = {};
  read(sockfd,list,sizeof(list));
  pf("list:%s\n",list);

  char filename[50] = {};
  pf("请输入要下载的文件名:");
  get_str(filename,50);
  write(sockfd,filename,strlen(filename)+1);

  char result[20] = {};
  read(sockfd,result,sizeof(result));
  pf("result:%s\n",result);

  sleep(1);

  if(strcmp(result,"文件存在") != 0)
  {
    pf("下载失败\n");
    getchar();

  }
  else
  { 
    int fd = open(filename,O_CREAT|O_RDWR,0777);
    int r_size = 0;
    int w_size = 0;
    do{
      usleep(500);
      char buf[1024] = {};
      r_size = read(sockfd,buf,sizeof(buf));
      pf("r_size:%d\n",r_size);
      w_size = write(fd,buf,r_size);
      pf("w_size:%d\n",w_size);

    }while(r_size==1024);
    sleep(1);
    pf("下载成功\n");
    close(fd);
    getchar();
  }
  return;

}

void list(void)
{
  char see[20] = "我想看你";
  write(sockfd,see,strlen(see)+1);

  char list[1024] = {};
  read(sockfd,list,sizeof(list));
  pf("list:%s\n",list);

  pf("输入cd+空格+目录，修改服务器工作目录，否则返回上一级\n");
  char cmd[50] = {};
  get_str(cmd,50);
  if(strstr(cmd,"cd ") == NULL)
  {
    pf("非cd指令，按任意键返回主界面\n");
    getchar();

  }
  else
  {
    char *dir = (char*)malloc(20);
    dir = strrchr(cmd,' ');
    dir += 1;
    write(sockfd,dir,strlen(dir)+1);

    read(sockfd,list,sizeof(list));
    pf("list:%s\n",list);
    getchar();

  }
  return;

}
