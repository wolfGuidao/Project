#include "HttpServer.hpp"
#include <signal.h>

int main(int argc,char* argv[])
{
  //if(argc!=3)
  //{
  //  std::cout << "Usage./diect" << std::endl;
  //  exit(2);
  //}
  signal(SIGPIPE,SIG_IGN);

  HttpServer s("0.0.0.0",9090);
  s.HttpInit();
  s.Start();
  return 0;
}
