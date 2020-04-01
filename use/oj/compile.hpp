#pragma once

#include <string>
#include <atomic>
#include <fstream>
#include <iostream>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>

#include <jsoncpp/json/json.h>

#include "util.hpp"

class Compiler {
private:
  // 对文件名字做出以下约定:
  // file_name(tmp_[id]): 文件名前缀.
  // file_name.cpp(tmp_[id].cpp): 源代码文件
  static std::string SrcPath(const std::string& file_name) {
    return "./tmp_files/" + file_name + ".cpp";
  }
  // file_name.compile_err(tmp_[id].compile_err): g++ 编译的出错信息
  static std::string CompileErrorPath(const std::string& file_name) {
    return "./tmp_files/" + file_name + ".compile_err";
  }
  // file_name.executable(tmp_[id].executable): g++ 生成的可执行程序
  static std::string ExePath(const std::string& file_name) {
    // 此处加上相对路径. 方便后面 exec 的时候就不用构造路径了.
    // 另外这里再强调一下, 这个可执行程序和 windows 的 exe 不一样.
    return "./tmp_files/" + file_name + ".executable";
  }
  // file_name.stdin(tmp_[id].stdin): 运行依赖的标准输入的内容
  static std::string StdinPath(const std::string& file_name) {
    return "./tmp_files/" + file_name + ".stdin";
  }
  // file_name.stdout(tmp_[id].stdout): 运行生成的可执行程序得到的标准输出结果.
  static std::string StdoutPath(const std::string& file_name) {
    return "./tmp_files/" + file_name + ".stdout";
  }
  // file_name.stderr(tmp_[id].stderr): 运行生成的可执行程序得到的标准错误结果.
  static std::string StderrPath(const std::string& file_name) {
    return "./tmp_files/" + file_name + ".stderr";
  }
  
public:
  // 最核心的入口函数!
  // 提供两个版本, 只是接口参数不同. 接口为 Json 或者为 std::string
  static void CompileAndRun(const Json::Value& req, Json::Value* resp) {
    // 1. 生成源代码文件
    // 先检查下 code 字段是否存在
    if (req["code"].empty()) {
      (*resp)["error"] = 3;
      LOG(ERROR) << "ParseReq failed! code empty!";
      return;
    }
    const std::string& code = req["code"].asString();
    std::string file_name = WriteTmpFile(code);
    if (file_name == "") {
      (*resp)["error"] = 4;
      LOG(ERROR) << "WriteTmpFile failed!";
      return;
    }
    // 2. 创建子进程, 调用 g++ 对源代码文件进行编译, 生成可执行程序
    if (!Compile(file_name)) {
      (*resp)["error"] = 1;

      std::string reason;
      FileUtil::ReadFile(CompileErrorPath(file_name), &reason);
      (*resp)["reason"] = reason;

      LOG(ERROR) << "Compile failed! check " << file_name;
      return;
    }
    // 4. 创建子进程, 执行可执行程序(通过重定向的方式写入标准输入的内容,
    //    并记录输出结果)
    int sig = Run(file_name, req["stdin"].asString());
    if (sig != 0) {
      (*resp)["error"] = 2;
      (*resp)["reason"] = "Program exit by sig " + std::to_string(sig);
      LOG(ERROR) << "Run error! check " << file_name << ", sig: "
                 << sig << "\n";
      return;
    }
    // 5. 构造响应内容
    (*resp)["error"] = 0;
    (*resp)["reason"] = "Compile and Run OK!";

    std::string stdout_result;
    FileUtil::ReadFile(StdoutPath(file_name), &stdout_result);
    (*resp)["stdout"] = stdout_result;

    std::string stderr_result;
    FileUtil::ReadFile(StderrPath(file_name), &stderr_result);
    (*resp)["stderr"] = stderr_result;

    // 6. 清理临时文件(测试阶段可以先不清理)
    // TODO 
    // Clean(file_name);
    return;
  }

private:

  // 这个函数需要生成临时文件名. 由于同时可能会并行执行多个编译任务, 不同的编译任务需要区分开
  static std::string WriteTmpFile(const std::string& code) {
    // 此处应该是一个静态的变量, 并且要能够保证线程安全.
    static std::atomic_uint id(0);
    ++id;
    // 文件名形如: tmp_1545299993.1
    std::string file_name = "tmp_" + std::to_string(TimeUtil::TimeStamp()) 
                            + "." + std::to_string(id);
    FileUtil::WriteFile(SrcPath(file_name), code);
    return file_name;
  }

  static bool Compile(const std::string& file_name) {
    // 1. 先构造好编译指令(g++ [file_name] -std=c++11 -o [file_name]_exe 2>[file_name]_compile_err)
    //    注意重定向部分不能直接构造字符串, 还是得借助 dup2.
    const int CommandCount = 20;
    char buf[CommandCount][50] = {{0}};
    char* command[CommandCount] = {0};
    for (int i = 0; i < CommandCount; ++i) {
      command[i] = buf[i];
    }
    sprintf(command[0], "%s", "g++");
    sprintf(command[1], "%s", SrcPath(file_name).c_str());
    sprintf(command[2], "%s", "-std=c++11");
    sprintf(command[3], "%s", "-o");
    sprintf(command[4], "%s", ExePath(file_name).c_str());
    sprintf(command[5], "%s", "-D");
    sprintf(command[6], "%s", "CompileOnline");
    command[7] = NULL;  // 一定要有一个 NULL 指针结尾
    // 2. 创建子进程
    int ret = fork();
    if (ret > 0) {
      // 3. 父进程进行进程等待. g++ 应该不会异常终止吧?
      waitpid(ret, NULL, 0);
    } else if (ret == 0) {
      // 4. 子进程进行重定向和程序替换(替换成刚才构造的指令)
      int fd = open(CompileErrorPath(file_name).c_str(),
          O_WRONLY | O_CREAT, 0666);
      if (fd < 0) {
        LOG(ERROR) << "open failed!\n";
        return false;
      }
      dup2(fd, 2);  // 此处照例要对着 man 手册琢磨一下
      execvp(command[0], command);
      exit(0);  // 这个 exit 至关重要, 如果替换失败, 就要让子进程销毁.
    } else {
      LOG(ERROR) << "fork failed!\n";
      return false;
    }
    // 5. 判定最终是否生成可执行程序. 如果生成成功, 则认为编译成功.
    struct stat st;
    ret = stat(ExePath(file_name).c_str(), &st);
    if (ret < 0) {
      LOG(ERROR) << "Compile failed! check " << file_name << "\n";
      return false;
    }
    return true;
  }

  // 返回值为子进程终止的信号
  static int Run(const std::string& file_name,
      const std::string& std_input) {
    // 1. 创建子进程.
    int ret = fork();
    if (ret == 0) {
      // [限制运行时间]
      // 注册闹钟, 1秒钟之后通过 闹钟信号 终止进程.
      // 通过这种方式限定 Oj 程序的执行时间. 
      alarm(1);
      // [限制内存使用]
      // #include <sys/resource.h>
      struct rlimit rlim;
      rlim.rlim_cur = 32768 * 1024;    // 内存限制 32768KB
      rlim.rlim_max = RLIM_INFINITY;   // 无限制
      setrlimit(RLIMIT_AS, &rlim);
      // 2. 子进程进行重定向(标准输入和标准输出都需要重定向)和程序替换
      //    此处的标准输入, 标准输出和标准错误采用文件方式处理, 方便进行调试或者后续归档.
      //    由于这是新鲜创建好的子进程, 因此打开文件就不检查出错了(捂脸)
      //    [注意] 此处不能用管道来重定向标准输入. 管道和普通文件终究还是存在差别,
      //    会导致客户端代码进行 std::cin 的时候不能够顺利的把数据读干净.
      //  a) 对标准输入重定向
      FileUtil::WriteFile(StdinPath(file_name), std_input);
      int fd_stdin = open(StdinPath(file_name).c_str(), O_RDONLY);
      dup2(fd_stdin, 0);
      //  b) 对标准输出和标准错误重定向
      int fd_stdout = open(StdoutPath(file_name).c_str(),
          O_WRONLY | O_CREAT, 0666);
      dup2(fd_stdout, 1);   // 往标准输出中写, 相当于写文件
      int fd_stderr = open(StderrPath(file_name).c_str(),
          O_WRONLY | O_CREAT, 0666);
      dup2(fd_stderr, 2);   // 往标准错误中写, 相当于写文件
      //  c) 进行程序替换
      execl(ExePath(file_name).c_str(), ExePath(file_name).c_str(), NULL);
      LOG(ERROR) << "exec failed!" << strerror(errno) << "\n";
      exit(0);
    }
    // 3. 父进程等待子进程结束(这里就需要判定异常终止的情况了)
    //    上面使用了 alarm 作为执行时间的控制. 也可以改成非阻塞轮询 更精细的处理超时问题
    //    最好把这个时间设定成可配置, 不同的题目要求执行不同的时间.
    int status = 0;
    waitpid(ret, &status, 0);
    return status & 0x7f;
  }

  static void Clean(const std::string& file_name) {
    unlink(SrcPath(file_name).c_str());
    unlink(CompileErrorPath(file_name).c_str());
    unlink(ExePath(file_name).c_str());
    unlink(StdinPath(file_name).c_str());
    unlink(StdoutPath(file_name).c_str());
    unlink(StderrPath(file_name).c_str());
  }
};
