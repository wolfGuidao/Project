#pragma once

///////////////////////////////////////////////////////
// 每一个 OJ 题目对应一个目录(放在 oj_data 下)
// 目录中包含了三个文件
// 1) header.cpp: .cpp 文件的上半部分, 主要是头文件包含 + 代码模板 + 用户要实现的代码主体
// 2) tail.cpp: .cpp 文件的末尾, 包含了测试用例代码和测试用例的执行过程(用例如何组织,
//    以及通过/失败的输出日志需要满足一定的约定)
// 3) desc.txt: 题目要求的详细描述.
//
// 另外还有一个总的配置文件, 是一个 行文本文件, 记录了每个题目的id, 标题, 路径, 难度信息
// (这个文件放到内存中).
//
// 该文件需要实现一个 OjModel 类, 提供以下接口:
// 1. 获取所有题目列表
// 2. 获取某个题目的题面信息(也就是只包含 oj1.header部分的信息)
// 3. 获取某个题目的完整信息(也就是 header + tail 拼接成的完整 cpp)
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
// tail.cpp 文件编写约定(此处可以考虑接入 gtest)
// 1. 每个用例是一个函数, 函数构造输入并获取校验输出.
// 2. 每个用例从标准输出输出一行日志
// 3. 如果用例通过, 统一打印 [TestName] ok!
// 4. 如果用例不通过, 统一打印 [TestName] failed! 并且给出合适的提示.
// 这样风格来设计, 可能对后期的多语言扩展不利. 但是不用引入额外的标准输入输出的配置文件,
// 实现风格更简洁好理解.
///////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>

#include "util.hpp"

struct Question {
  std::string id;       // 题目的 id
  std::string title;    // 题目的标题
  std::string dir;      // 题目对应的目录
  std::string star;     // 题目难度
};

class OjModel {
private:
  static std::string HeaderPath(const std::string& question_dir) {
    return question_dir + "/header.cpp";
  }
  static std::string TailPath(const std::string& question_dir) {
    return question_dir + "/tail.cpp";
  }
  static std::string DescPath(const std::string& question_dir) {
    return question_dir + "/desc.txt";
  }
public:
  OjModel () {
    assert(Load("./oj_config.cfg"));
  }

  OjModel(const OjModel&) = delete;
  OjModel& operator=(const OjModel&) = delete;

  bool GetAllQuestions(std::vector<Question>* questions) const {
    for (const auto& kv : model_) {
      questions->push_back(kv.second);
    }
    // 再来个排序吧, 按照 id 升序. 如果是想按照其他顺序排序
    // 只要调整 lambda 的实现细节即可.
    std::sort(questions->begin(), questions->end(),
        [](const Question& l, const Question& r) {
          return std::stoi(l.id) < std::stoi(r.id);
        });
    return true;
  }

  bool GetQuestionBrief(const std::string& id, Question* question, std::string* code,
      std::string* desc) const {
    // 1. 根据 id 找到题目的具体信息
    auto it = model_.find(id);
    if (it == model_.end()) {
      // 该 id 对应的题目不存在
      LOG(ERROR) << "Question not found! id=" << id << "\n";
      return false;
    }
    *question = it->second;
    // 2. 根据题目路径加载 header.cpp 
    bool ret = FileUtil::ReadFile(HeaderPath(question->dir), code);
    if (!ret) {
      LOG(ERROR) << "Load question code failed! id=" << question->id << "\n";
      return false;
    }
    // 3. 根据题目路径加载 desc.txt
    ret = FileUtil::ReadFile(DescPath(question->dir), desc);
    if (!ret) {
      LOG(ERROR) << "Load question desc failed! id=" << question->id << "\n";
      return false;
    }
    return true;
  }

  bool GetQuestionDetail(const std::string& id, const std::string& user_code,
      std::string* code) const {
    // 1. 根据 id 找到题目的具体信息
    auto it = model_.find(id);
    if (it == model_.end()) {
      // 该 id 对应的题目不存在
      LOG(ERROR) << "Question not found! id=" << id << "\n";
      return false;
    }
    const Question& question = it->second;
    // 2. 根据题目路径加载 tail.cpp
    std::string tail;
    bool ret = FileUtil::ReadFile(TailPath(question.dir), &tail);
    if (!ret) {
      LOG(ERROR) << "Load question tail failed! id=" << question.id << "\n";
      return false;
    }
    // 4. 填充结果
    *code = user_code + tail;
    return true;
  }
private:
  std::unordered_map<std::string, Question> model_;

  bool Load(const std::string& config_path) {
    std::ifstream file(config_path.c_str());
    if (!file.is_open()) {
      return false;     
    }
    std::string line;
    while (std::getline(file, line)) {
      // 针对 line 进行切分, 字段用 \t 切分
      std::vector<std::string> tokens;
      StringUtil::Split(line, "\t", &tokens);
      // 跳过出错的行
      if (tokens.size() != 4) {
        continue;
      }
      Question question;
      question.id = tokens[0];
      question.title = tokens[1];
      question.dir = tokens[2];
      question.star = tokens[3];
      model_[question.id] = question;
    }
    file.close();
    LOG(INFO) << "Load " << model_.size() << " questions!\n";
    return true;
  }
};
