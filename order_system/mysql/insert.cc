#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>

int main()
{
  //1.创建操作数据库的句柄mysql
  MYSQL* mysql = mysql_init(NULL);

  //2.建立句柄和数据库之间的联系
  if(mysql_real_connect(mysql,"127.0.0.1","root","","order_system",3306,NULL,0)==NULL)
  {
    printf("connect failed!  %s\n",mysql_error(mysql));
    return 1;
  }

  //3.设置编码格式(让mysql服务器的编码方式和客户端的编码方式一致)
  mysql_set_character_set(mysql,"utf8");

  //4.拼装sql语句
  char sql[1024] = {0};
  int price = 2000;
  sprintf(sql,"insert into dish_table values(NULL,'肉丝',%d)",price);

  //5.执行sql语句
  int ret = mysql_query(mysql,sql);
  if(ret!=0)
  {
    printf("sql 执行失败！%s\n",mysql_error(mysql));
    mysql_close(mysql);
    return 1;
  }

  //6.关闭连接
  mysql_close(mysql);

  return 0;
}
