#include <stdio.h>
#include <sys/stat.h>
#include <unordered_map>
#include <queue>
#include <iostream>
typedef bool (*Handler)(int sock);
//using namespace std;

//任务类
class HttpTask
{
  //http请求的任务
  //包含一个成员就是socket
  //包含一个任务处理函数
  private:
    int _cli_sock;
    Handler TaskHandler;
#if 1
  public:
    void SetHttpTask(int sock,Handler handler)
    {
      _cli_sock = sock;
      TaskHandler = handler;
    }

    void Handler()
    {
      TaskHandler(_cli_sock);
    }
#endif
};


//线程池
class ThreadPoool
{
  //创建指定数量的线程
  //创建线程安全的任务队列
  //提供任务的入队、出队、线程池销毁等接口
  private:
    int _max_thr;//线程的最大数量
    std::queue<HttpTask>_task_queue;//任务队列
    pthread_mutex_t _mutex;//线程安全
    pthread_cond_t _cond;//线程同步
    int _cur_thr;//当前线程池中的线程数

  private:
    //完成线程获取任务并执行任务
    static void* thr_start(void* arg);

  public:
    ThreadPoool(int max)
      :_max_thr(max)
    {

    }

    //完成线程创建及互斥锁、条件变量的初始化
    bool ThreadPooolInit();


    //线程安全的任务入队
    bool PushTask(HttpTask& tt);


    //线程安全的任务出队
    bool PodTask(HttpTask& tt);

    //线程池的销毁
    bool ThreadPoolStop();

};

//服务器
class HttpServer
{
  //建立一个tcp服务端程序，接收新连接
  //为新链接组织一个线程池任务，添加到线程池中
  private:
    int _serv_sock;
    ThreadPoool* _tp; 

  private:
    static bool (HttpHandler)(int sock);//http任务处理函数

  public:
    
    //完成tcp服务端sock、线程池的初始化
    bool TcpServerInit();

    //程序入口。开始获取客户端新连接---创建任务，任务入队
    bool Start();
};

class RequestInfo
{
  //包含HttpRequest解析的请求信息
  public:
    std::string _method;//请求方法
    std::string _version;//协议版本
    std::string _path_info;//资源路径
    std::string _path_phys;//请求资源的实际路经
    std::string _query_string;//查询字符串
    std::unordered_map<std::string,std::string>_hdr_list;//存放头部信息
    //int stat(const char *restrict path, struct stat *restrict buf);
    struct stat _st;///获取文件信息

  public:
};

//请求
class HttpRequest
{
  //http数据的接收
  //http数据的解析
  //对外提供能够获取解析结果的接口
  private:
    int _cli_sock;
    RequestInfo _req_info;
    std::string _http_head;

  public:
    HttpRequest()
      :_cli_sock(-1)
    {

    }

    //接收http请求头
    bool RecvHttpHead();

    //解析
    bool ParseHttpHead();

    //返回解析结果
    RequestInfo &  GetRequestinfo();
};

class HttpResponse
{
  //文件请求接口(完成文件下载/列表功能)
  //CGI请求接口（完成文件上传功能）
  private:
    int _cli_sock;
    std::string _etag;//标记文件是否修改过
    std::string _mtime;//最后修改的时间
    std::string _cont_len;//content长度

  public:
    bool InitResponse(RequestInfo _req_info);//初始化请求的响应信息

    bool ProcessFile(std::string & file);//文件下载

    bool ProcessList(std::string & path);//文件列表

    bool ProcessCGI(std::string & file);//处理CGI请求

};

//上传文件
class UpLoad
{
  //提供上传文件处理接口
};

//工具类
class Utils
{
  //提供公用的功能接口
};
