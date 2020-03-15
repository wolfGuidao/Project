#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>

int main()
{
  //1.创建mysql句柄
  MYSQL* mysql = mysql_init(NULL);

  //2.建立链接
  if(mysql_real_connect(mysql,"127.0.0.1","root","","order_system",3306,NULL,0)==NULL)
  {
    printf("mysql建立链接失败！%s\n",mysql_error(mysql));
    return 1;
  }

  //3.设置编码方式
  mysql_set_character_set(mysql,"utf8");

  //4.拼装sql语句
  char sql[1024]={0};
  sprintf(sql,"select * from dish_table");

  //5.执行sql
  int ret = mysql_query(mysql,sql);
  if(ret !=0)
  {
    printf("mysql执行失败！%s\n",mysql_error(mysql));
    mysql_close(mysql);
    return 1;
  }

  //6.获取并遍历结果集合
  //a.获取结果集合
  MYSQL_RES* result = mysql_store_result(mysql);

  //b.获取结果中的行数
  int rows = mysql_num_rows(result);
  int cols = mysql_num_fields(result);

  //c.遍历结果
  for(int row = 0;row<rows;row++)
  {
    //得到一行数据
    MYSQL_ROW mysql_row = mysql_fetch_row(result);

    //获取每一列数据
    for(int col = 0;col < cols;col++)
    {
      printf("%s\t",mysql_row[col]);
    }
    printf("\n");
  }

  //7.结果集合需要手动释放
  mysql_free_result(result);

  //8.关闭mysql,断开连接！
  mysql_close(mysql);

  return 0;
}
