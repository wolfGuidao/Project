#include <jsoncpp/json/json.h>
#include "../include/httplib.h"
#include "../base/util.hpp"
#include "compiler.hpp"
#include "oj_model.hpp"
#include "oj_view.hpp"

void test()
{
    using namespace httplib;
    // 创建一个http服务器
    httplib::Server server;
    // 创建model对象
    ojModel model;
    // 为 model 对象加载题目列表
    model.load();
    // 为 http 服务器 注册一个处理get请求的方法
    // 这个 get 请求将拿到所有题目列表
    server.Get("/all_questions", [&model](const httplib::Request& req,
                                    httplib::Response& resp){
               // 由于请求的是 all_questions 所以忽略参数
               (void)req;
               // 调用 model 模块的 getAllQuestion 方法，拿到所有题目
               std::vector<Question> all_questions;
               model.getAllQuestion(&all_questions);

               // 通过 view 模块 渲染http响应
               std::string html;
               ojView::renderAllQuestions(all_questions, &html);
               // 将http响应写会浏览器端
               resp.set_content(html, "text/html");
               });
    // 为 http 服务器注册一个处理 get 请求的方法
    // 这个 get 请求将拿到具体题目的详细信息
    // R"()"  c++11 原始字符串 -- 忽略字符串的转移字符
    // \d+    正则表达式 用一些特殊符号来表示字符串满足啥样条件
    server.Get(R"(/question/(\d+))", [&model](const httplib::Request& req, 
                               httplib::Response& resp){
               Question question;
               // 调用 model 模块的 getQuestion 方法拿到指定id对应的题目详细信息
               model.getQuestion(req.matches[1].str(), &question);
               std::string html;
               // 通过 view 模块 渲染 http 响应
               ojView::renderQuestion(question, &html);
               resp.set_content(html, "text.html");
               });

    // 为 http 服务器注册一个 post 方法
    // 这个 post 方法为编译指定题目，并讲编译结果返回给浏览器
    server.Post(R"(/compile/(\d+))", [&model](const httplib::Request& req, 
                                              httplib::Response& resp){
                // 1. 先根据 id 获取到题目信息
                Question question;
                model.getQuestion(req.matches[1].str(), &question);

                // 2. 解析 body ，获取到用户提交的代码
                std::unordered_map<std::string, std::string> bodyKV;
                util::urlUtil::parseBody(req.body, &bodyKV);
                const std::string& userCode = bodyKV["code"];

                // 3. 构造 JSON 结构的参数
                Json::Value req_json;
                // 真实需要编译的代码，是用户提交的代码+题目的测试用例代码
                req_json["code"] = userCode + question.tail_cpp;
                Json::Value resp_json;

                // 4. 调用编译模块进行编译 编译成功就运行
                // error,0 正确编译运行，1 编译出错 ， 2 运行出错, 3 其他错误
                int ret = compiler::compilerAndRun(req_json, &resp_json);
                std::string html;
                if(ret == 0) // 0. 正确编译运行
                {
                    // 5. 根据最终结果构造最终的网页
                    // TODO 如果编译出错了 这里没有处理
                    ojView::renderResult(resp_json["stdout"].asString(), 
                                         resp_json["reason"].asString(), 
                                         resp_json["stderr"].asString(), &html);
                }
                else if (ret == 1) // 1. 编译出错
                {
                    // 6. 编译出错了
                    std::string html;
                    const std::string& reason = resp_json["reason"].asString();
#if __DEBUG_ON__
                    LOG(util::DEBUG) << "reason = " << reason << std::endl;
#endif
                    ojView::renderCompileErrorResult(resp_json["error"].asString(), reason
                                         , &html);
                }
                else if (ret == 2) // 2. 运行出错
                {

                    std::string html;
                    ojView::renderRunErrorResult(resp_json["error"].asString(),
                                                 resp_json["reason"].asString(), &html);
                }
                else 
                {
                    // 其他未知错误
                    ojView::renderUnknownErrorResult(resp_json["error"].asString(),
                                                     resp_json["reason"].asString(), &html);
                }
                resp.set_content(html, "text/html");
                });

    server.Get("/loginPage", [](const httplib::Request& req, httplib::Response& resp){
               (void)req;
               std::string html;
               ojView::showLoginPage(&html);
               resp.set_content(html, "text/html");
               });

    server.Post("/login", [](const httplib::Request& req, httplib::Response& resp){
                std::string username = req.matches[1].str();
                std::cout << "username = " << username << std::endl;
                std::string html = "<html><h1>登录成功</h1></html>";
                resp.set_content(html, "text/html");
                });

    // 设置 http 服务器的静态页面
    server.set_base_dir("./systemPage");
    // 设置服务器的监听模式 端口为 9092
    // 公网ip是 47.102.208.185
    server.listen("0.0.0.0", 9092);
}

int main()
{
    test();

    return 0;
}
