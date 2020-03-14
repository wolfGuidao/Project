#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include "../base/util.hpp"
// model 模块负责加载文件
struct Question
{
    std::string id;
    std::string name;
    std::string dir; // 
    std::string star;

    std::string desc;
    std::string header_cpp;
    std::string tail_cpp;
};

class ojModel
{
public:

    bool getAllQuestion(std::vector<Question>* questions) const
    {
        questions->clear();
        // 遍历哈希表
        for(const auto& kv : _model)
        {
            questions->push_back(kv.second);
        }

        return true;
    }

    bool getQuestion(const std::string& id, Question* q) const
    {
        std::map<std::string, Question>::const_iterator it = _model.find(id);

        if(it == _model.end())
        {
            return false;
        }
        *q = it->second;

        return true;
    }

    // 加载到内存
    bool load()
    {
        // 1. 先打开 oj_config 文件
        std::ifstream file("./oj_data/oj_config.cfg");
        if(!file.is_open())
        {
            return false;
        }
        // 2. 按行读取，并且解析
        std::string line;
        while (std::getline(file, line))
        {
            std::vector<std::string> tokens;
            util::stringUtil::split(line, "\t", &tokens);
            if(tokens.size()!=4)
            {
                LOG(util::ERROR) << "config file format error" << std::endl;
                continue;
            }
            Question q;
            q.id = tokens[0];
            q.name = tokens[1];
            q.star = tokens[2];
            q.dir = tokens[3];
            util::fileUtil::read(q.dir + "/desc.txt", &q.desc);
            util::fileUtil::read(q.dir + "/header.cpp", &q.header_cpp);
            util::fileUtil::read(q.dir + "/tail.cpp", &q.tail_cpp);
            _model[q.id] = q;
        }
        // 3. 根据解析结果拼装成Question结构体
        // 4. 插入哈希表中
        file.close();
        LOG(util::INFO) << "load " << _model.size() << " questions" << std::endl;
        return true;
    }
private:
    std::map<std::string, Question> _model;
    // 哈希函数
    // MD5
    // SHA1
    // 1. 计算的哈希值非常均匀
    // 2. 不可逆，通过字符串算哈希值很容易，但是通过hash值计算原串极难
    // 3. 固定长度，不管字符串多长，得到的哈希值都是固定长度
};
