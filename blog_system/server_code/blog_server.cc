#include"httplib.h"
#include<signal.h>
#include"db.hpp"


MYSQL* mysql=NULL;

int main()
{
    using namespace httplib;
    using namespace blog_system;

    //1.先和数据库建立链接
    mysql = blog_system::MySQLInit();
    //因为第三方库封装的listen是一个死循环，正常的MySQLRelease无法执行到，所以采用信号的方式来结束
    signal(SIGINT,[](int){blog_system::MySQLRelease(mysql);
            exit(0);
            });

    //2.创建相关数据库处理对象
    BlogTable blog_table(mysql);
    TagTable tag_table(mysql);

    //3.创建服务器,并设置'路由(HTTP中的路由)'；这里的路由指：方法 + path 映射到那个处理函数
    Server server;

    //新增博客
    server.Post("/blog",[&blog_table](const Request& req,Response& resp){

            printf("新增博客！\n"); 
            //1.因为用户的请求是HTTP版本的，所以应获取请求中的body并解析成json
            Json::Reader reader;
            Json::Value req_json;
            Json::Value resp_json;
            Json::FastWriter writer;

            //进行解析,把待解析的字符串req.body解析到输出型参数req_json中
            bool ret = reader.parse(req.body,req_json);
            if(!ret)
            {
            //解析失败，如客户传入的不是json形式
            printf("解析请求失败！%s\n",req.body.c_str());

            //因为不只是让服务器端知道解析失败，还得让客户端知道解析失败，构造一个响应对象
            //告诉客户端请求失败，
            resp_json["ok"]=false;
            resp_json["reason"]="input data parse error!\n";
            resp.status=400;//设置状态码
            resp.set_content(writer.write(resp_json),"application/json");//第二个参数是json的数据类型
            return ;
            }

            //2.用户传来的请求虽然解析成功，但json不一定合法，对参数进行校验
            if(req_json["title"].empty()||
                    req_json["content"].empty()||
                    req_json["tag_id"].empty()||
                    req_json["create_time"].empty())
            {
                printf("请求数据格式有错！%s\n",req.body.c_str());
                //告诉客户端请求失败，
                resp_json["ok"]=false;
                resp_json["reason"]="input data format error!";
                resp.status=400;//设置状态码
                resp.set_content(writer.write(resp_json),"application/json");//第二个参数是json的数据类型
                return ;
            }

            //3.调用MYSQL接口进行操作
            ret = blog_table.Insert(req_json);
            if(!ret)
            {
                printf("博客插入失败！\n");
                resp_json["ok"]=false;                                                                   
                resp_json["reason"]="blog Insert failed!";                                        
                resp.status=500;//设置状态码
                resp.set_content(writer.write(resp_json),"application/json");//第二个参数是json的数据类型
                return ; 
            }

            //4.构造一个正确的插入博客响应给客户端
            printf("博客插入成功！\n");
            resp_json["ok"]=true;                                                                   
            //resp.status=200;//设置状态码
            resp.set_content(writer.write(resp_json),"application/json");//第二个参数是json的数据类型
            return ;

    });

    //查看所有博客列表
    server.Get("/blog",[&blog_table](const Request& req,Response& resp){
            Json::FastWriter writer;

            printf("查看所有博客！\n");
            //1.尝试获取tag_id;如果tag_id不存在，返回空字符串
            const std::string& tag_id = req.get_param_value("tag_id");

            //不需要解析请求相关操作

            //2.调用数据库操作来获取参看结果
            Json::Value resp_json;
            bool ret = blog_table.SelectAll(&resp_json,tag_id);
            if(!ret)
            {
            printf("查看所有博客失败！\n");
            resp_json["ok"]=false;
            resp_json["reason"]="select all failed";
            resp.status=500;
            resp.set_content(writer.write(resp_json),"application/json");
            return ;
            }

            //3.构造查看所有博客成功的响应结果
            resp.set_content(writer.write(resp_json),"application/json");
            return ;

    });

    //查看某个博，要用到正则表达式,下面的R是为了解决\转义好看的问题（\\太难看，c++11支持的用法R显示
    //      原始字符串）;其中(\d+):是一个正则表达式代表一个数字
    server.Get(R"(/blog/(\d+))",[&blog_table](const Request& req,Response& resp){
            Json::FastWriter writer;

            //1.解析获取blog_id---->req.matches[1]
            //  printf("%s,%s\n",req.matches[0].str().c_str(),req.matches[1].str().c_str());
            int32_t blog_id = std::stoi(req.matches[1].str());
            printf("查看id为%d的博客！\n",blog_id);

            //2.调用数据库操作
            Json::Value resp_json;
            bool ret = blog_table.SelectOne(blog_id,&resp_json);
            if(!ret)
            {
            printf("查看id为%d号博客失败！\n",blog_id);
            resp_json["ok"]=false;
            resp_json["reason"]="select one failed";
            resp.status=404;
            resp.set_content(writer.write(resp_json),"application/json");
            return ;
            }

            //3.正确的响应
            resp.set_content(writer.write(resp_json),"application/json");
            return ;
    });

    //修改某个博客
    server.Put(R"(/blog/(\d+))",[&blog_table](const Request& req,Response& resp){

            //1.先获取blog_id
            int32_t blog_id = std::stoi(req.matches[1].str());
            printf("修改id为%d的博客！\n",blog_id);

            //2.获取请求并解析
            Json::Reader reader;
            Json::FastWriter writer;
            Json::Value resp_json;
            Json::Value req_json;
            bool ret = reader.parse(req.body,req_json);
            if(!ret)
            {
            printf("修改id为%d号博客失败！\n",blog_id);
            resp_json["ok"]=false;
            resp_json["reason"]="update blog parse request failed";
            resp.status=400;
            resp.set_content(writer.write(resp_json),"application/json");
            return ;
            }

            //3.校验参数是否符合预期
            if(req_json["title"].empty()||
                    req_json["content"].empty()||
                    req_json["tag_id"].empty())
            {
                printf("修改博客格式有错！%s\n",req.body.c_str());
                //告诉客户端修改失败，
                resp_json["ok"]=false;
                resp_json["reason"]="update blog request format error!";
                resp.status=400;//设置状态码
                resp.set_content(writer.write(resp_json),"application/json");//第二个参数是json的数据类型
                return ;
            }

            //4.调用数据库相关操作,因为blog_id是从用户请求的path中获得的，req_json本身是没有的
            //   而Update函数内部用到了blog_id所以得把blog_id插入到req_json中
            req_json["blog_id"]=blog_id;
            ret = blog_table.Update(req_json); 
            if(!ret)
            {
                resp_json["ok"]=false;
                resp_json["reason"]="update blog database failed";
                resp.status=500;
                resp.set_content(writer.write(resp_json),"application/json");
                return ;
            }

            //5.构造正确的返回结果
            resp_json["ok"]=true;
            resp.set_content(writer.write(resp_json),"application/json");
            return ;
    });

    //删除某个博客
    server.Delete(R"(/blog/(\d+))",[&blog_table](const Request& req,Response& resp){
            Json::FastWriter writer;
            Json::Value resp_json;

            //1.获取到blog_id
            int32_t blog_id = std::stoi(req.matches[1].str());
            printf("删除id为%d的博客\n",blog_id);

            //2.调用数据库操作
            bool ret = blog_table.Delete(blog_id);
            if(!ret)
            {
            printf("执行删除博客失败！\n");
            resp_json["ok"]=false;
            resp_json["reason"]="delete blog  failed";
            resp.status=500;
            resp.set_content(writer.write(resp_json),"application/json");
            return ;
            }

            //3.构造正确的响应
            resp_json["ok"]=true;
            resp.set_content(writer.write(resp_json),"application/json");

            return ;
    });

    //新增标签
    server.Post("/tag",[&tag_table](const Request& req,Response& resp){
            Json::FastWriter writer;
            Json::Value req_json;
            Json::Value resp_json;
            Json::Reader reader;

            //1.解析请求
            bool ret = reader.parse(req.body,req_json);
            if(!ret)
            {
            printf("插入标签失败！\n");
            resp_json["ok"]=false;
            resp_json["reason"]="insert tag req parse failed";
            resp.status=400;
            resp.set_content(writer.write(resp_json),"application/json");
            return ;

            }

            //2.对请求进行校验
            if(req_json["tag_name"].empty())
            {
                printf("插入博客标签有错！%s\n",req.body.c_str());
                //告诉客户端修改失败，
                resp_json["ok"]=false;
                resp_json["reason"]="insert blog tag format error!";
                resp.status=400;//设置状态码
                resp.set_content(writer.write(resp_json),"application/json");//第二个参数是json的数据类型
            } 

            //3.调用数据库操作
            ret = tag_table.Insert(req_json);
            if(!ret)
            {
                printf("插入标签失败！\n");
                resp_json["ok"]=false;
                resp_json["reason"]="insert tag req parse failed";
                resp.status=500;
                resp.set_content(writer.write(resp_json),"application/json");
                return ;

            }

            //4.返回正确的结果
            resp_json["ok"]=true;
            resp.set_content(writer.write(resp_json),"application/json");
            return ;
    });

    //删除标签
    server.Delete(R"(/tag/(\d+))",[&tag_table](const Request& req,Response& resp){
            Json::FastWriter writer;
            Json::Value req_json;
            Json::Value resp_json;
            Json::Reader reader;

            //1.解析请求，获取tag_id
            int32_t tag_id = std::stoi(req.matches[1].str());
            printf("删除id为%d的标签！\n",tag_id);

            //2.执行数据库操作
            bool ret = tag_table.Delete(tag_id);
            if(!ret)
            {
            printf("删除标签失败！\n");
            resp_json["ok"]=false;
            resp_json["reason"]="delete tag failed";
            resp.status=500;
            resp.set_content(writer.write(resp_json),"application/json");
            return ;
            }

            //3.构造正确的响应
            resp_json["ok"]=true;
            resp.set_content(writer.write(resp_json),"application/json");
            return ;
    });

    //查看所有标签
    server.Get("/tag",[&tag_table](const Request& req,Response& resp){
            (void)req;

            Json::FastWriter writer;
            Json::Value resp_json;

            //1.直接执行数据库部分
            bool ret  = tag_table.SelectAll(&resp_json);
            if(!ret)
            {
            printf("获取标签失败！\n");
            resp_json["ok"]=false;
            resp_json["reason"]="select all tag failed";
            resp.status=500;
            resp.set_content(writer.write(resp_json),"application/json");
            return ;
            }

            //2.构造正确的响应结果
            resp.set_content(writer.write(resp_json),"application/json");
            return ;

    });

    server.set_base_dir("./wwwroot");
    server.listen("0.0.0.0",9093);
    return 0;
}
