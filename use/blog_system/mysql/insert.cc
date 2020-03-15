//用MySql API实现数据的插入功能
#include <cstdio>
#include <cstdlib>
#include <time.h>
//编译器默认从/usr/include目录中查找
#include<mysql/mysql.h>

int main()
{
  //1.创建一个数据库链接的句柄
  MYSQL* connect_fd = mysql_init(NULL);

  //2.和数据库建立链接（在应用层建立链接）
  //  链接过程需要指定一些必要的信息：
  //  a）链接句柄
  //  b）服务器的ip（直接设为环回IP）
  //  c）用户名
  //  d）密码
  //  e）数据库名（blog_system）
  //  f）服务器的端口号:默认端口号3306
  //  g）unix_sock:NULL
  //  h）client_flag：0
  if(mysql_real_connect(connect_fd,"127.0.0.1","root","","blog_system",3306,NULL,0)==NULL)
  {
    printf("链接失败！%s\n",mysql_error(connect_fd));
    //mysql_close(connect_fd);
    return 1;
  }
  printf("链接成功！\n");

  //3.设置编码方式
  //      mysql server默认为utf8
  //      客户端编码方式也得utf8
  mysql_set_character_set(connect_fd,"utf8");

  //4.拼接SQL语句
  char sql[1024*10]={0};
  char title[]="test";
  char content[]="这不知道是一个什么正文";
  int tag_id=1;
#if 0
  time_t rawtime;
  struct tm *ptminfo;
   time(&rawtime);
  ptminfo = localtime(&rawtime);
  char* str;
  sprintf(str,"%02d-%02d-%02d %02d:%02d:%02d", ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);
  string strr = 
#endif
  char date[]="\"2020-01-10 20:20:20\"";
  sprintf(sql,"insert into blog_table values(null,'%s','%s',%d,'%s')",title,content,tag_id,date);
  printf("sql:%s\n",sql);

  //5.让数据库服务器执行sql
  int ret = mysql_query(connect_fd,sql);
  if(ret<0)
  {
    printf("执行sql失败!%s\n",mysql_error(connect_fd));
    mysql_close(connect_fd);
    return 1;
  }

  printf("插入成功！\n");

  //6.断开链接，关闭数据库
  mysql_close(connect_fd);
  return 0;
}
