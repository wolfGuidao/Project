#pragma once
#include <iostream>
#include <string>
#include <atomic>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <jsoncpp/json/json.h>
#include "../base/util.hpp"

#define TMP_FILE "./tmp_file/"

class compiler
{
public:

    // 1. 源代码文件
    static std::string srcPath(const std::string& name)
    {
        std::string ret = TMP_FILE + name + ".cpp";
#if __DEBUG_ON__
        LOG(util::DEBUG) << "srcPath = " << ret << std::endl;
#endif
        return ret;
    }
    // 2. 编译出错文件
    static std::string compileErrorPath(const std::string& name)
    {
        std::string ret = TMP_FILE + name + ".compileError";
#if __DEBUG_ON__
        LOG(util::DEBUG) << "compileErrorPath = " << ret << std::endl;
#endif
        return ret;
    }
    // 3. 可执行程序文件
    static std::string exePath(const std::string& name)
    {
        std::string ret = TMP_FILE + name + ".exe";
#if __DEBUG_ON__
        LOG(util::DEBUG) << "exePath = " << ret << std::endl;
#endif
        return ret;
    }
    // 4. 标准输入文件
    static std::string stdinPath(const std::string& name)
    {
        std::string ret = TMP_FILE + name + ".stdin";
#if __DEBUG_ON__
        LOG(util::DEBUG) << "stdinPath = " << ret << std::endl;
#endif
        return ret;

    }
    // 5. 标准输出文件
    static std::string stdoutPath(const std::string& name)
    {
        std::string ret = TMP_FILE + name + ".stdout";
#if __DEBUG_ON__
        LOG(util::DEBUG) << "stdoutPath = " << ret << std::endl;
#endif
        return ret;
    }
    // 6. 标准错误文件
    static std::string stderrPath(const std::string& name)
    {
        std::string ret = TMP_FILE + name + ".stderr";
#if __DEBUG_ON__
        LOG(util::DEBUG) << "stderrPath = " << ret << std::endl;
#endif
        return ret;
    }

    // error,0 正确编译运行，1 编译出错 ， 2 运行出错, 3 其他错误("code" = "")
    static int compilerAndRun(const Json::Value& req/* 输入型参数 */, Json::Value* resp/* 输出型参数 */)
    {
        // 1. 根据请求对象，生成源代码文件
        if(req["code"].empty())
        {
            (*resp)["error"] = 3;
            (*resp)["reason"] = "code empty";
            LOG(util::INFO) << "code empty" << std::endl;
            return 3;
        }
        const std::string& code = req["code"].asString();
#if __DEBUG_ON__
        LOG(util::DEBUG) << "code = " << code << std::endl;
#endif
        std::string fileName = writeTmpFile(code);
#if __DEBUG_ON__
        LOG(util::DEBUG) << "filename = " << fileName << std::endl;
#endif
 
        if(!req["stdin"].empty())
        {
            // TODO: 假如某道题有输入怎么办？
            ;
        }
        // 2. 调用g++进行编译(fork + exec) 生成可执行程序
        //    如果编译出错，需要把编译错误记录下来(重定向标准错误到文件)
        bool ret = compile(fileName);
        if(!ret)
        {
            (*resp)["error"] = 1;
            std::string reason;
            std::string compileFileName = compileErrorPath(fileName);
            LOG(util::INFO) << fileName << " compile error" << std::endl;
#if __DEBUG_ON__
            LOG(util::DEBUG) << "compileFileName = " << compileFileName << std::endl;
#endif
            util::fileUtil::read(compileFileName, &reason);
            (*resp)["reason"] = reason;
#if __DEBUG_ON__
            LOG(util::DEBUG) << "reason = " << reason << std::endl;
#endif
            return 1; // 编译出错
        }
        // 3. 调用可执行程序，把标准输入记录到文件，然后把文件中内容重定向到可执行程序
        //    可执行程序的标准输出和标准错误也需要重定向记录到文件中
        int signo = run(fileName);
        if(signo != 0)
        {
            (*resp)["error"] = 2;
            (*resp)["reason"] = "program exit by signo: " + std::to_string(signo);
            LOG(util::INFO) << "program exit by signo: " + std::to_string(signo) << std::endl;
            return 2; // 运行出错
        }
        // 4. 把程序的最终结果，构造resp对象进行返回
        (*resp)["error"] = 0;
        (*resp)["reason"] = "";
        std::string strStdout;
        util::fileUtil::read(stdoutPath(fileName), &strStdout);
        (*resp)["stdout"] = strStdout;
        std::string strStderr;
        util::fileUtil::read(stderrPath(fileName), &strStderr);
        (*resp)["stderr"] = strStderr;
        LOG(util::INFO) << "program " << fileName << " Done" << std::endl;

        return 0; // 程序正确运行得到结果
    }
private:
    static std::string writeTmpFile(const std::string& code)
    {
        std::string fileName;
        static std::atomic_int count(0);
        ++count;
        fileName += "tmp_" + std::to_string(util::timeUtil::getTimeStamp()) + '.' + std::to_string(count);
        util::fileUtil::write(srcPath(fileName), code);
        return fileName;
    }

    static bool compile(const std::string& fileName)
    {
        char* command[20] = {0};
        char buf[20][50] = {{0}};
        for(size_t i = 0; i < 20; ++i)
        {
            command[i] = buf[i];
        }
        sprintf(command[0], "%s", "g++");
        sprintf(command[1], "%s", srcPath(fileName).c_str());
        sprintf(command[2], "%s", "-o");
        sprintf(command[3], "%s", exePath(fileName).c_str());
        sprintf(command[4], "%s", "-std=c++11");
        command[5] = NULL;
        // 创建子进程
        int ret = fork();
        if(ret == 0)
        {
            // 子进程
            int fd = open(compileErrorPath(fileName).c_str(), O_WRONLY | O_CREAT, 0666);
            if(fd < 0)
            {
                LOG(util::ERROR) << "open file error" << std::endl;
                exit(1);
            }
            dup2(fd, 2);
            execvp(command[0], command);
            exit(0); // 子进程替换失败，直接退出
        }
        else if(ret > 0)
        {
            // 父进程
            waitpid(ret, NULL, 0);
        }
        else
        {
            // 子进程创建失败
            LOG(util::ERROR) << "fork failed" << std::endl;
            return false;
        }
        struct stat st;
        ret = stat(exePath(fileName).c_str(), &st);
        if(ret < 0)
        {
            LOG(util::INFO) << "compile failed " << fileName << std::endl;
            return false;
        }

        LOG(util::INFO) << "compile " << fileName << " ok" << std::endl;
        return true;
    } // end compile

    static int run(const std::string& fileName)
    {
        int ret = fork();
        if(ret == 0)
        {
            // 子进程
            int fdStdin = open(stdinPath(fileName).c_str(), O_RDONLY);
            dup2(fdStdin, 0);
            int fdStdout = open(stdoutPath(fileName).c_str(), O_WRONLY | O_CREAT, 0666);
            dup2(fdStdout, 1);
            int fdStderr = open(stderrPath(fileName).c_str(), O_WRONLY | O_CREAT, 0666);
            dup2(fdStderr, 2);
            execl(exePath(fileName).c_str(), exePath(fileName).c_str(), NULL);
            exit(0);
        }
        else if (ret > 0)
        {
            // 父进程
            int status = 0;
            waitpid(ret, &status, 0);
            return status&0x7f;
        }
        else
        {
            // 子进程创建失败
            LOG(util::ERROR) << "fork failed" << std::endl;
            return 0;
        }
    }// end run
};
