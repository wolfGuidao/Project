#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "../include/httplib.h"

void server(void)
{
    httplib::Server server;

    std::ifstream filein("./hello.html", std::fstream::in);
    char buf[1024] = {0};
    filein.read(buf, 1024);
    size_t len = strlen(buf) ;
    std::cout << "buf len = " << len << " , buf = " << buf << std::endl;


    server.Get("/all", [&buf](httplib::Request req, httplib::Response resp){
               (void)req;
                std::string html(buf, strlen(buf));
                std::cout << "html = " << html;
                resp.set_content(html, "text/html");
                });

    server.set_base_dir(".");
    server.listen("0.0.0.0", 9652);
}


int main(int argc, char* argv[])
{
    if(argc > 1)
    {
        // TODO: 考虑守护进程
        (void)argv;
    }



    // 创建服务器
    server();

    return 0;
}
