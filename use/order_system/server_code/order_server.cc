#include <jsoncpp/json/json.h>
#include "until.hpp"
#include <signal.h>
#include "db.hpp"
#include <memory>
#include "httplib.h"
#include <iostream>
using namespace std;

const char* CONTENT_TYPE="application/json";
MYSQL* mysql=NULL;

int main()
{
  using namespace httplib;

  mysql = order_system::MYSQLInit();
  signal(SIGINT,[](int){
      order_system::MYSQLRelease(mysql);
      exit(0);
      });
  order_system::DishTable dish_table(mysql);
  order_system::OrderTable order_table(mysql);
  
  printf("server is ok!\n");

  Server server;
  //新增菜品
  server.Post("/dish",[&dish_table](const Request& req,Response& resp){
      Json::Value req_json;
      Json::Value resp_json;
      Json::Reader reader;
      Json::FastWriter writer;
      //std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
      //std::unique_ptr<Json::CharReader>reader(Json::CharReaderBuilder().newCharReader());
      printf("新增菜品！\n");
      //1.获取到数据并解析成json格式
      bool ret = reader.parse(req.body,req_json); 
      if(!ret)
      {
      //如果失败构造响应的响应给客户端
        printf("parse body is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="parse body is failed!";
        resp.status=400;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return;
      }

      //2.校验json中的数据信息
      if(req_json["name"].empty()||req_json["price"].empty())
      {
        printf("parse request is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="parse request is failed!";
        resp.status=400;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;
      }

      //3.调用数据库操作
      ret = dish_table.Insert(req_json);
      if(!ret)
      {
        printf("dishtable insert is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="dihstable insert is failed!";
        resp.status=500;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      }
      //4.构造正确的响应
        printf("dishtable insert is ok!\n");
        resp_json["ok"]=true;
        resp.status=200;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      });

  //查看所有菜品 
  server.Get("/dish",[&dish_table](const Request& req,Response& resp){
      (void)req;
      printf("查看所有菜品！\n");
      Json::Value req_json;
      Json::Value resp_json;
      Json::Reader reader;
      Json::FastWriter writer;
      //std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
      //std::unique_ptr<Json::CharReader>reader(Json::CharReaderBuilder().newCharReader());
      
      bool ret = dish_table.SelectAll(&resp_json);
      if(!ret)
      {
        printf("selectall dish is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="selectall dish is failed!";
        resp.status=500;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      }
      //2.构造正确的响应
        printf("dishtable selectall is ok!\n");
        resp.status=200;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      });

  //查看某个菜品，要用正则表达式
  server.Get(R"(/dish/(\d+))",[&dish_table](const Request& req,Response& resp){
      Json::Value req_json;
      Json::Value resp_json;
      Json::Reader reader;
      Json::FastWriter writer;

      //1.获取菜品id
      int32_t dish_id = std::stoi(req.matches[1]);
      printf("获取到菜品编号为%d的菜品\n",dish_id);

      //2.执行数据库操作
      bool ret = dish_table.SelectOne(dish_id,&resp_json);
      if(!ret)
      {
        printf("selectone dish is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="selectone dish is failed!";
        resp.status=500;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      }

      //3.构造正确的响应
        printf("dishtable selectone is ok!\n");
        resp_json["ok"]=true;
        resp.status=200;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      });

  //修改菜品
  server.Put(R"(/dish/(\d+))",[&dish_table](const Request& req,Response& resp){
      Json::Value req_json;
      Json::Value resp_json;
      Json::Reader reader;
      Json::FastWriter writer;
      //std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
      //std::unique_ptr<Json::CharReader>reader(Json::CharReaderBuilder().newCharReader());
      printf("修改菜品！\n");

      //1.获取菜品id
      int32_t dish_id = std::stoi(req.matches[1]);
      printf("修改菜品编号为%d的菜品\n",dish_id);


      //2.获取到数据并解析成json格式
      bool ret = reader.parse(req.body,req_json); 
      if(!ret)
      {
      //如果失败构造响应的响应给客户端
        printf("parse body is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="parse body is failed!";
        resp.status=400;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return;
      }

      //2.校验json中的数据信息
      if(req_json["name"].empty()||req_json["price"].empty())
      {
        printf("parse request is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="parse request is failed!";
        resp.status=400;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;
      }

      //3.调用数据库操作
      req_json["dish_id"]=dish_id;//json中本来没有dish_id，需要手动加上

      ret = dish_table.Update(req_json);
      if(!ret)
      {
        printf("dishtable update is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="dihstable update is failed!";
        resp.status=500;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      }
      //4.构造正确的响应
        printf("dishtable update is ok!\n");
        resp_json["ok"]=true;
        resp.status=200;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      });
  
  //删除菜品
  server.Delete(R"(/dish/(\d+))",[&dish_table](const Request& req,Response& resp){
      Json::Value req_json;
      Json::Value resp_json;
      Json::Reader reader;
      Json::FastWriter writer;
      //std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
      //std::unique_ptr<Json::CharReader>reader(Json::CharReaderBuilder().newCharReader());
      
      //1.获取菜品id
      int32_t dish_id = std::stoi(req.matches[1]);
      printf("删除菜品编号为%d的菜品\n",dish_id);

      //2.执行数据据操作
      bool ret = dish_table.Delete(dish_id);
      if(!ret)
      {
        printf("dishtable Delete is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="dihstable delete is failed!";
        resp.status=500;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      }

      //3.构造正确的响应
        printf("dishtable delete is ok!\n");
        resp_json["ok"]=true;
        resp.status=200;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;
      });

  //新增订单
  server.Post("/order",[&order_table](const Request& req,Response& resp){
      Json::Value req_json;
      Json::Value resp_json;
      Json::Reader reader;
      Json::FastWriter writer;
      //std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
      //std::unique_ptr<Json::CharReader>reader(Json::CharReaderBuilder().newCharReader());
      //1.获取body数据并解析
      bool ret = reader.parse(req.body,req_json);
      if(!ret)
      {
        printf("ordertable insert is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="ordertable insert is failed!";
        resp.status=400;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      }

      //2。进行校验
      if(req_json["table_id"].empty()||
          req_json["time"].empty()||
          req_json["dish_id"].empty()
      )
      {
        printf("ordertable insert is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="ordertable insert is failed!";
        resp.status=400;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      }

      //3.构造其他字段
      req_json["status"]=1;//1表示订单在进行中，0表示订单关闭
      
      //需要把dishids（json）字段转换成dishes字符串
      const Json::Value& dish_ids = req_json["dish_ids"];
      req_json["dishes"]=writer.write(dish_ids);

      //4.调用数据库操作
      
      ret = order_table.Insert(req_json);
      if(!ret)
      {
        printf("ordertable insert is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="ordertable insert is failed!";
        resp.status=500;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      }

      //5.构造正确的响应
        printf("dishtable insert is ok!\n");
        resp_json["ok"]=true;
        resp.status=200;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;
      
  });



  //查看订单
  server.Get("/order",[&order_table](const Request& req,Response& resp){
      (void)req;
      printf("获取所有订单：\n");
      Json::Value req_json;
      Json::Value resp_json;
      Json::Reader reader;
      Json::FastWriter writer;
      //std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
      //std::unique_ptr<Json::CharReader>reader(Json::CharReaderBuilder().newCharReader());
      
      //1.执行数据库操作，获取响应信息
      bool ret = order_table.SelectAll(&resp_json);
      if(!ret)
      {
        printf("ordertable selectall is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="selectall insert is failed!";
        resp.status=500;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      }

      
      //2.构造正确的响应
        printf("dishtable ieselectall is ok!\n");
        resp.status=200;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

      });

  //修改订单
  server.Put(R"(/order/(\d+))",[&order_table](const Request& req,Response& resp){
      Json::Value req_json;
      Json::Value resp_json;
      Json::Reader reader;
      Json::FastWriter writer;
      //std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
      //std::unique_ptr<Json::CharReader>reader(Json::CharReaderBuilder().newCharReader());
      //1.获取order_id
      int32_t order_id = std::stoi(req.matches[1]);
      printf("修改订单编号为%d的订单状态\n",order_id);

      //2.解析请求中的status
     bool ret = reader.parse(req.body,req_json);
     if(!ret)
     {
        printf("ordertable update is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="update insert is failed!";
        resp.status=500;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

     }

     //3.校验json
     if(req_json["status"].empty())
     {
        printf("ordertable update is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="update insert is failed!";
        resp.status=400;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

     }

     //4.执行数据库操作
     req_json["order_id"]=order_id;
     ret = order_table.ChangeStatus(req_json);
     if(!ret)
     {
        printf("ordertable changestatus is failed!\n");
        resp_json["ok"]=false;
        resp_json["reason"]="update changestatus is failed!";
        resp.status=500;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;

     }

     //5.构造正确的响应
        printf("dishtable changestatus is ok!\n");
        resp_json["ok"]=true;
        resp.status=200;
        resp.set_content(writer.write(resp_json),CONTENT_TYPE);
        return ;
     
      });

  //对index.html进行修改，从query_string获取table_id填到页面的{{table_id}}中
  server.Get("/table",[](const Request& req,Response& resp){
      const std::string& table_id = req.get_param_value("table_id");
      printf("table_id:%s\n",table_id.c_str());

      std::string html;
      FileUtil::ReadFile("./wwwroot/index.html",&html);
      std::string html_out;
      StringUtil::Replace(html,"{{table_id}}",table_id,&html_out);
      resp.set_content(html_out,CONTENT_TYPE);
      });

  //让服务器读取./wwwroot目录中的文件
  server.set_base_dir("./wwwroot");

  server.listen("0.0.0.0",9094);

  return 0;
}
