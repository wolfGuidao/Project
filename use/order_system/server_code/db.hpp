#pragma once
#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>
#include<jsoncpp/json/json.h>

namespace order_system
{
  static MYSQL* MYSQLInit()
  {
    //1.创建mysql句柄
    MYSQL* mysql = mysql_init(NULL);

    //2.建立链接
    if(mysql_real_connect(mysql,"127.0.0.1","root","","order_system",3306,NULL,0)==NULL)
    {
      printf("mysql链接失败！%s\n",mysql_error(mysql));
      return NULL;
    }

    //3.设置编码方式
    mysql_set_character_set(mysql,"utf8");
    return mysql;

  }

  static void MYSQLRelease(MYSQL* mysql)
  {
    mysql_close(mysql);
  }

  class DishTable
  {
    public:
      DishTable(MYSQL* mysql)
        :mysql_(mysql)
      {

      }

      //Json::Value表示一个JSON对象，用法和std:map很相似,是一个键值对
      //Writer::writer方法表示把json对象序列化成字符串（std::string）
      //Reaser::parse:表示把字符串反序列化为json对象
      bool Insert(const Json::Value& dish)
      {
        //拼装sql
        char sql[1024*10]={0};
        sprintf(sql,"insert into dish_table values(null,'%s',%d)",
            dish["name"].asCString(),dish["price"].asInt());

        //执行sql
        int ret = mysql_query(mysql_,sql);
        if(ret!=0)
        {
          printf("dishtable Insert is failedi %s\n",mysql_error(mysql_));
          return false;
        }
        printf("dishtable Insert is ok\n");
        return true;
      }

      //dishes表示输出型参数
      //const & 表示输入型参数
      //* 表示输出型参数
      //& 表示输入输出型参数
      bool SelectAll(Json::Value* dishes)
      {
        char sql[1024*10]={0};
        sprintf(sql,"select dish_id, name, price from dish_table");
        int ret = mysql_query(mysql_,sql);
        if(ret!=0)
        {
          printf("dishtable selectall is faild! %s",mysql_error(mysql_));
          return false;
        }
        printf("dishtabel selectall is ok!\n");

        //构造结果
        MYSQL_RES* result = mysql_store_result(mysql_);
        int rows = mysql_num_rows(result);
        for(int i=0 ;i<rows ;  i++)
        {
          MYSQL_ROW row = mysql_fetch_row(result);
          Json::Value dish;
          //注意和数据库统一
          dish["dish_id"]=atoi(row[0]);
          dish["name"]=row[1];
          dish["price"]=atoi(row[2]);

          dishes->append(dish);
        }

        //注意释放结果集合
        mysql_free_result(result);
        return true;
      }

      bool SelectOne(int32_t dish_id,Json::Value* dish)
      {

        //1.拼sql语句
        char sql[1024*10]={0};
        sprintf(sql,"select dish_id,name,price from dish_table where dish_id =%d",dish_id);

        //2.执行sql语句
        int ret = mysql_query(mysql_,sql);
        if(ret!=0)
        {
          printf("dishtable selectne is failed! %s\n",mysql_error(mysql_));
          return false;
        }

        //3.遍历结果集合
        MYSQL_RES* result = mysql_store_result(mysql_);
        int rows = mysql_num_rows(result);
        if(rows!=1)
        {
          printf("dishtable selectone result is failed!\n");
          //注意这个出错时也需要回收result
          mysql_free_result(result);
          return false;
        }

        MYSQL_ROW row = mysql_fetch_row(result);
        (*dish)["dish_id"]=dish_id;
        (*dish)["name"]=row[1];
        (*dish)["price"]=atoi(row[2]);

        mysql_free_result(result);

        return true;
      }

      bool Update(const Json::Value& dish)
      {
        //1.拼装sql
        char sql[1024*10]={0};
        sprintf(sql,"update dish_table set name ='%s',price = %d where dish_id =%d",
            dish["name"].asCString(),
            dish["price"].asInt(),
            dish["dish_id"].asInt()
            );
        printf("sql:%s\n",sql);

        //2.执行sql
        int ret = mysql_query(mysql_,sql);
        if(ret!=0)
        {
          printf("update dishtable failed! %s\n",mysql_error(mysql_));
          return false;
        }

        printf("update dishtable is ok!\n");

        return true;
      }

      bool Delete(int32_t dish_id)
      {
        //1.拼装sql
        char sql[1024*10]={0};
        sprintf(sql,"delete from dish_table where dish_id =%d",
            dish_id
            );
        printf("sql:%s\n",sql);

        //2.执行sql
        int ret = mysql_query(mysql_,sql);
        if(ret!=0)
        {
          printf("delete dishtable failed! %s\n",mysql_error(mysql_));
          return false;
        }

        printf("delete dishtable is ok!\n");

        return true; 

      }

    private:
      MYSQL* mysql_;
  };

  class OrderTable
  {
    public:
      OrderTable(MYSQL* mysql)
        :mysql_(mysql)
      {

      }

      bool Insert(const Json::Value& order)
      {
        //1.拼装sql
        char sql[1024*10]={0};
        sprintf(sql,"insert into order_table Values(null,'%s','%s','%s',%d)",
            order["table_id"].asCString(),
            order["time"].asCString(),
            order["dishes"].asCString(),
            order["status"].asInt()
            );

        //2.执行sql
        int ret = mysql_query(mysql_,sql);
        if(ret!=0)
        {
          printf("order insert is failed! %s\n",mysql_error(mysql_));
          return false;
        }
        printf("order insert is ok! \n");

        return true;
      }

      bool SelectAll(Json::Value* orders)
      {
        //1.拼装sql
        char sql[1024*10]={0};
        sprintf(sql,"select order_id,table_id,time,dishes,status from order_table");

        //2.执行sql
        int ret = mysql_query(mysql_,sql);
        if(ret!=0)
        {
          printf("order selectall is failed! %s\n",mysql_error(mysql_));
          return false;
        }

        //3.拼装当orders中
        MYSQL_RES* result = mysql_store_result(mysql_);
        int rows = mysql_num_rows(result);
        for(int i=0;i<rows;i++)
        {
          MYSQL_ROW row =mysql_fetch_row(result);
          Json::Value order;
          order["order_id"]=atoi(row[0]);
          order["table_id"]=row[1];
          order["time"]=row[2];
          order["dishes"]=row[3];
          order["status"]=atoi(row[4]);
          orders->append(order);
        }

        mysql_free_result(result);
        printf("order selectall is ok! \n");
        return true;
      }

      bool ChangeStatus(const Json::Value& order)
      {

        char sql[1024*10]={0};
        sprintf(sql,"update order_table set status =%d where order_id =%d",
            order["status"].asInt(),
            order["order_id"].asInt());

        //2.执行sql
        int ret = mysql_query(mysql_,sql);
        if(ret!=0)
        {
          printf("order ChangeStatus is failed! %s\n",mysql_error(mysql_));
          return false;
        }

        printf("ordertable changestatus is ok!\n");
        return true;
      }

    private:
      MYSQL* mysql_;

  };
}//order_system is end
