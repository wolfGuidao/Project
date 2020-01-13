#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>

int main()
{
    //1.创建一个句柄
    MYSQL* connect_fd = mysql_init(NULL);

    //2.建立链接
    if(mysql_real_connect(connect_fd,"127.0.0.1","root","","blog_system",3306,NULL,0)==NULL)
    {
        printf("链接失败！%s\n",mysql_error(connect_fd));
        //mysql_close(connect_fd);
        return 1;
    }
    printf("链接成功！\n");

    //3.设置编码方式
    mysql_set_character_set(connect_fd,"utf8");

    //4.拼接SQL语句
    char sql[1024*10]="select * from blog_table";

    //5.执行SQL语句
    int ret = mysql_query(connect_fd,sql);
    if(ret<0)
    {
        printf("出错了！%s\n",mysql_error(connect_fd));
        mysql_close(connect_fd);
        return 1;
    }

    //6.遍历结果集合,MYSQL_RES select 得到的结果集合
    MYSQL_RES* result = mysql_store_result(connect_fd);

    //a)获取到结果集合中的行数和列数
    int rows = mysql_num_rows(result);
    int fields = mysql_num_fields(result);
    //b)根据行数和列数来遍历结果
    for(int i=0;i<rows;i++)
    {
        //调用一次获取到一行数据
        MYSQL_ROW row = mysql_fetch_row(result);
        for(int j=0;j<fields;j++)
        {
            printf("%s\t",row[j]);
        }
        printf("\n");
    }

    //释放结果集合,不要忘记
    mysql_free_result(result);

    mysql_close(connect_fd);
    return 0;
}
