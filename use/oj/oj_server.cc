#include "httplib.h"
#include "util.hpp"
#include "compile.hpp"
#include "oj_model.hpp"
#include "oj_view.hpp"

int main() {
  using namespace httplib;
  // 1. 加载 model 数据
  OjModel model;
  // 2. 设定路由(路由指的是每一个 URL 的 PATH 对应的处理函数是什么)
  //    此处需要设定三种路由
  //    a) 获取所有的问题列表
  //    b) 获取某个问题的详细页面
  //    c) 提交编译并获取结果(失败几个用例)
  Server server;
  server.Get("/all_questions", [&model](const Request& req, Response& resp) {
      (void) req;
      std::vector<Question> questions;
      model.GetAllQuestions(&questions);
      std::string html;
      OjView::RenderAllQuestions(questions, &html);
      resp.set_content(html, "text/html");
    });

  // raw string(c++ 11), 转义字符不生效. 用来表示正则表达式正好合适
  // 关于正则表达式, 只介绍最基础概念即可. \d+ 表示匹配一个数字
  // 语法参考 <<正则表达式30分钟入门教程>>
  // http://help.locoy.com/Document/Learn_Regex_For_30_Minutes.htm
  server.Get(R"(/question/(\d+))", [&model](const Request& req, Response& resp) {
      // 用这个代码来验证 req.matches 的结果是啥
      // LOG(INFO) << req.matches[0] << "," << req.matches[1] << "\n";
      std::string code;
      std::string desc;
      Question question;
      model.GetQuestionBrief(req.matches[1].str(), &question, &code, &desc);
      std::string html;
      OjView::RenderQuestion(question, code, desc, &html);
      resp.set_content(html, "text/html");
    });

  server.Post(R"(/compile/(\d+))", [&model](const Request& req, Response& resp) {
      // 1. 根据请求获取到用户编写的代码
      std::unordered_map<std::string, std::string> params;
      UrlUtil::ParseBody(req.body, &params);
      const std::string user_code = params["code"];
      // 2. 根据题目编号, 拼装出完整的可编译的代码
      std::string code;
      model.GetQuestionDetail(req.matches[1].str(), user_code, &code);
      // 3. 交给编译模块进行编译
      Json::Value req_json;
      req_json["code"] = code;
      req_json["stdin"] = "";
      Json::Value resp_json;
      Compiler::CompileAndRun(req_json, &resp_json);
      // 4. 根据编译结果构造最终响应
      const std::string& case_result = resp_json["stdout"].asString();
      const std::string& reason = resp_json["reason"].asString();
      std::string html;
      OjView::RenderCompileResult(case_result, reason, &html);
      resp.set_content(html, "text/html");
    });

  // 设定 http 服务器的根目录
  server.set_base_dir("./wwwroot");
  LOG(INFO) << "Server Start!\n";
  server.listen("0.0.0.0", 9090);
  return 0;
}
