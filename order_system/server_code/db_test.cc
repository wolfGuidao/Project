#include "db.hpp"
#include<iostream>
#include<memory>
using namespace std;

void TestDishTable()
{
  MYSQL* mysql = order_system::MYSQLInit();
  order_system::DishTable dish_table(mysql);
  std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());

  //1.插入数据
#if 0
  Json::Value dish;
  dish["name"]="宫保鸡丁";
  dish["price"]=1000;

  bool ret = dish_table.Insert(dish);
  printf("%d\n",ret);
#endif
  
  //2.查找所有
#if 0
  Json::Value dishes;
  int ret = dish_table.SelectAll(&dishes);
  printf("%d\n",ret);
  writer->write(dish, &std::cout);
#endif

  //3.查找指定
#if 0
  Json::Value dish;
  int ret = dish_table.SelectOne(1,&dish);
  printf("%d\n",ret);
  writer->write(dish, &std::cout);
#endif

  //4.修改
#if 0
  Json::Value dish;
  dish["dish_id"]=1;
  dish["name"]="女朋友";
  dish["price"]=99999;

  int ret = dish_table.Update(dish);
  printf("%d\n",ret);
#endif

  //删除
#if 0
  int ret = dish_table.Delete(6);
   dish_table.Delete(7);
   dish_table.Delete(8);
   dish_table.Delete(9);
   dish_table.Delete(10);
   dish_table.Delete(11);
   dish_table.Delete(12);
   dish_table.Delete(13);
   dish_table.Delete(14);
   dish_table.Delete(15);
   dish_table.Delete(16);
   dish_table.Delete(17);
   dish_table.Delete(18);
   dish_table.Delete(19);
   dish_table.Delete(20);
   dish_table.Delete(21);
   dish_table.Delete(22);
  printf("%d\n",ret);
#endif

  order_system::MYSQLRelease(mysql);
}

void TestOrderTable()
{
  MYSQL* mysql = order_system::MYSQLInit();
  order_system::OrderTable order_table(mysql);
  std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());

  //1.插入订单
#if 0
  Json::Value order;
  order["table_id"]="丈母娘";
  order["time"]="2020/01/01 12:30";
  order["dishes"]="[1,2,3]";
  order["status"]=1;

  int ret = order_table.Insert(order);
  printf("%d\n",ret);
#endif

  //2,查看所有订单
  Json::Value orders;
  int ret = order_table.SelectAll(&orders);
  printf("%d\n",ret);
  writer->write(orders, &std::cout);

  //3.修改状态
  Json::Value order;
  order["order_id"]=5;
  order["status"]=0;

  int ret1 = order_table.ChangeStatus(order);
  printf("%d\n",ret1);

  order_system::MYSQLRelease(mysql);
}

int main()
{
  //TestDishTable();
  TestOrderTable();
  return 0;
}
