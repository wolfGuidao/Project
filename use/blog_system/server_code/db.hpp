//创建一些类封装数据库操作
#include<memory>
#include<cstring>
#include<jsoncpp/json/json.h>
#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>


namespace blog_system
{
    static MYSQL* MySQLInit()
    {
        //初始化一个MySQL句柄并建立链接
        
        //1.创建一个句柄
        MYSQL* connect_fd = mysql_init(NULL);

        //2.和数据库建立链接
       if(mysql_real_connect(connect_fd,"127.0.0.1","root","","blog_system",3306,NULL,0)==NULL) 
       {
           printf("链接失败! %s\n",mysql_error(connect_fd));
           return NULL;
       }

       printf("server is ok!\n");
       //3.设定字符编码
       mysql_set_character_set(connect_fd,"utf8");
       return connect_fd;
    }

    static void MySQLRelease(MYSQL* connect_fd)
    {
        //释放句柄并断开链接
        mysql_close(connect_fd);
    }
        
    //创建一个用于操作博客表的类
    class BlogTable
    {
        public:
            BlogTable(MYSQL* connect_fd)
            :mysql_(connect_fd)
            {
                //通过构造函数获取到数据库的句柄
                
            }


        //以下操作的参数统一用JSON的方式
        //Json::Value 表示一个具体的json对象
        //如
        //{
        //    title:xxx
        //    content:xxx
        //    treate_time:xxx
        //    tag_id:xxx
        //}
        //方便扩展
            bool Insert(const Json::Value& blog)
            {
                //获取正文
                const std::string& content = blog["content"].asString();

                //把正文进行转义，解决单引号等在拼接字符串时的匹配问题
                //文档要求to的缓冲空间大小为content.size()*2+1;且string型的content是临时定义的，不能直接传
                //      content.size()*2+1，需要动态分配,注意释放空间
                //char* to=new char[content.size()*2+1];这样写释放空间是比较麻烦的，容易漏掉,采用智能指针来解决
                std::unique_ptr<char> to(new char[content.size()*2+1]);//不用手动去释放空间
                
                mysql_real_escape_string(mysql_,to.get(),content.c_str(),content.size());

                //拼装SQL语句,拼装时也一样，不知道具体的大小，同理得使用智能指针来解决
                std::unique_ptr<char> sql(new char[content.size()*2+4096]);
                sprintf(sql.get(),"insert into blog_table values(null,'%s','%s',%d,'%s')",
                        blog["title"].asCString(),
                        to.get(),
                        blog["tag_id"].asInt(),
                        blog["create_time"].asCString());

                int ret = mysql_query(mysql_,sql.get());
                if(ret!=0)
                {
                    printf("执行插入博客失败！ %s\n",mysql_error(mysql_));
                    return false;
                }
                printf("执行插入博客成功！\n");
                return true;
            }

            //输出型参数：blogs,返回查找结果
            bool SelectAll(Json::Value* blogs,const std::string& tag_id="")
            {
                //因为查找不需要太长的sql，所以定长就ok，不用动态分配
                char sql[1024*4]={0};
                if(tag_id=="")
                {
                    //如果tag_id等于"",说明此时不需要按照tag_id来筛选结果
                    sprintf(sql,"select blog_id,title,tag_id,create_time from blog_table");
                }
                else
                {
                    //此时就需要按照tag_id来筛选
                    sprintf(sql,"select blog_id,title,tag_id,create_time from blog_table where tag_id = %d"
                            ,std::stoi(tag_id));
                }

                int ret = mysql_query(mysql_,sql);
                if(ret!=0)
                {
                    printf("执行查找所有博客失败！%s\n",mysql_error(mysql_));
                    return false;
                }
                
                //通过该函数获取结果集合，进行遍历,把结果写道blogs参数中，返回调用者
                MYSQL_RES* result = mysql_store_result(mysql_);
                //获取每一行
                int rows = mysql_num_rows(result);
                for(int i=0;i<rows;i++)
                {
                    //获取每一列
                    MYSQL_ROW row = mysql_fetch_row(result);
                    Json::Value blog;
                    //row[]中的下标和select中的顺序是一致的
                    blog["blog_id"] = atoi(row[0]);
                    blog["title"]=row[1];
                    blog["tag_id"] = atoi(row[2]);
                    blog["create_time"]= row[3];

                    //因为博客能容不只一行，所以得用append
                    blogs->append(blog);
                }
                //注意需要释放
                mysql_free_result(result);
                printf("执行查找所有博客成功！共查找到%d条博客\n",rows);

                return true;
            }

            //输出型参数：根据blog_id在数据库中找到具体的博客内容并通过blog返回给调用者
            bool SelectOne(int32_t blog_id,Json::Value* blog)
            {
               char sql[1024]={0};
               sprintf(sql,"select blog_id,title,content,tag_id,create_time from blog_table where blog_id = %d",blog_id);
               int ret = mysql_query(mysql_,sql);
                if(ret!=0)
                {
                    printf("执行查找博客失败！%s\n",mysql_error(mysql_));
                    return false;
                }
                MYSQL_RES* result = mysql_store_result(mysql_);
                int rows = mysql_num_rows(result);
                if(rows!=1)
                {
                    printf("查找到的博客不只有一条，实际有 %d 条！\n",rows);
                    return false;
                }
                MYSQL_ROW row = mysql_fetch_row(result);
                (*blog)["blog_id"]=atoi(row[0]);
                (*blog)["title"]=row[1];
                (*blog)["content"]=row[2];
                (*blog)["tag_id"]=atoi(row[3]);
                (*blog)["creat_time"]=row[4];

                return true;
            }

            bool Update(const Json::Value& blog)
            {
                const std::string& content = blog["content"].asString();
                std::unique_ptr<char> to(new char[content.size()*2+1]);//不用手动去释放空间
               //转义 
                mysql_real_escape_string(mysql_,to.get(),content.c_str(),content.size());
                
                std::unique_ptr<char> sql(new char[content.size()*2+4096]);
                sprintf(sql.get(),"update blog_table set title='%s',content='%s',tag_id=%d where blog_id =%d",
                        blog["title"].asCString(),
                        to.get(),
                        blog["tag_id"].asInt(),
                        blog["blog_id"].asInt());

                int ret = mysql_query(mysql_,sql.get());
                if(ret!=0)
                {
                    printf("更新博客失败！%s\n",mysql_error(mysql_));
                    return false;
                }
                printf("更新博客成功！\n");
                return true;
            }

            bool Delete(int32_t blog_id)
            {
                char sql[1024*4]={0};
                sprintf(sql,"delete from blog_table where blog_id =%d",blog_id);

                int ret = mysql_query(mysql_,sql);
                if(ret!=0)
                {
                    printf("删除博客失败！%s",mysql_error(mysql_));
                    return false;
                }
                printf("删除成功！\n");
                return true;
            }

        private:
            MYSQL* mysql_;
    };

    class TagTable
    {
        public:
            TagTable(MYSQL* mysql)
            :mysql_(mysql)
            {

            }

        bool Insert(const Json::Value& tag)
        {
            char sql[1024*4];
            sprintf(sql,"insert into tag_table values(null,'%s')",tag["tag_name"].asCString());
            int ret = mysql_query(mysql_,sql);
            if(ret!=0)
            {
                printf("插入标签失败！%s\n",mysql_error(mysql_));
                return false;
            }
            printf("插入标签成功！\n");
            return true;
        }

        bool Delete(int32_t tag_id)
        {
            char sql[1024*4]={0};
            sprintf(sql,"delete from tag_table where tag_id =%d",tag_id);
            int ret = mysql_query(mysql_,sql);
            if(ret!=0)
            {
                printf("删除标签失败！%s\n",mysql_error(mysql_));
                return false;
            }
            printf("删除标签成功！\n");
            return true;
        }

        bool SelectAll(Json::Value* tags)
        {
            char sql[1024*4]={0};
            sprintf(sql,"select tag_id, tag_name from tag_table");
            int ret = mysql_query(mysql_,sql);
            if(ret!=0)
            {
                printf("查找标签失败！%s\n",mysql_error(mysql_));
                return false;
            }
            MYSQL_RES* result = mysql_store_result(mysql_);
            int rows=mysql_num_rows(result);
            for(int i=0;i<rows;i++)
            {
                MYSQL_ROW row = mysql_fetch_row(result);
                Json::Value tag;
                tag["tag_id"]=atoi(row[0]);
                tag["tag_name"]=row[1];
                tags->append(tag);
            }
            printf("查找成功！共找到%d个标签!\n",rows);

            return true;
        }

        private:
        MYSQL* mysql_;
    };
}//blog_system is end
