#include "httplib.h"
#include "compile.hpp"
#include "util.hpp"

int main() {
  using namespace httplib;
  Server server;
  server.Post("/compile", [](const Request& req, Response& resp) {
      // 1. 先进行参数格式的转换
      std::unordered_map<std::string, std::string> body_kv;
      UrlUtil::ParseBody(req.body, &body_kv);
      Json::Value req_json;
      for (const auto& p : body_kv) {
        req_json[p.first] = p.second;
      }
      // 2. 进行编译运行
      Json::Value resp_json;
      Compiler::CompileAndRun(req_json, &resp_json);
      Json::FastWriter writer;
      resp.set_content(writer.write(resp_json), "application/json");
      LOG(INFO) << "[req] " << req.body << " [resp] " << resp.body << "\n";
  });
  // 设定 http 服务器的根目录
  server.set_base_dir("./wwwroot");
  LOG(INFO) << "Server Start!\n";
  server.listen("0.0.0.0", 9090);
  return 0;
}
