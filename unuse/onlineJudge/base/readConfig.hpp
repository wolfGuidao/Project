/*
 * fileName: ReadConfig.hpp
 * 核心类是 ReadConfig 负责读取配置文件 并解析  存储在 map 里
 * */
#pragma once

#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include <memory>

class ReadConfig
{
public:
    // 构造函数 加载配置文件
    ReadConfig(const char* fileName)
    {
        loadFile(fileName);
    }

    ~ReadConfig()
    {}

    // 将根据 key 获取对应的 value
    char* getConfigName(const char* name)
    {
        if(!_isReadOk)
        {
            return nullptr;
        }

        char* value = nullptr;
        // 此处完全可以用 auto 代替
        // 但为了更加直观，我们直接用的是 全称
        std::map<std::string, std::string>::iterator it = _config_map.find(name);
        if(it != _config_map.end())
        {
            value = (char*)it->second.c_str();
        }
        return value;
    }

    // 设置某个配置的值
    int setConfigValue(const char* name, const char* value)
    {
        if(!_isReadOk)
        {
            return -1;
        }

        std::map<std::string, std::string>::iterator it = _config_map.find(name);
        if(it != _config_map.end())
        {
            it->second = value;
        }
        else
        {
            _config_map.insert(std::make_pair(name, value));
        }

        return writeFile();
    }
private:
    // 负责加载配置文件，读取每一行，解析，并将解析出来的键值对存储在 map 中
    void loadFile(const char* fileName)
    {
        // 操作之前，先将 map 清空
        _config_map.clear();
        // 保存文件名
        _config_file.append(fileName);
        // 以只读方式打开文件
        FILE* fp = ::fopen(fileName, "r");
        if(!fp)
            return;
        char buf[256];
        for(;;)
        {
            // C库的fgets函数可以读取一行
            char* p = ::fgets(buf, 256, fp);
            if(!p)
                break;

            size_t len = ::strlen(buf);
            if(buf[len-1] == '\n')
                buf[len-1] = '\0'; // remove \n at the end

            // 我们约定配置文件的注释是 '#' 开头的字符串
            char* ch = strchr(buf, '#'); // remove string start with # 
            if(ch)
                *ch = '\0';

            // 如果这行是空行或者是被处理过的注释，直接跳过
            if(::strlen(buf) == 0)
                continue;

            // 解析一行
            parseLine(buf);
        }

        ::fclose(fp);
        _isReadOk = true;
    }

    int writeFile(const char* fileName = nullptr)
    {
        FILE* fp = nullptr;
        if(fileName == nullptr)
        {
            fp = ::fopen(_config_file.c_str(), "w");
        }
        else
        {
            fp = ::fopen(fileName, "w");
        }

        if(fp == nullptr)
        {
            return -1;
        }

        char szPair[128] = {0};
        std::map<std::string, std::string>::iterator it = _config_map.begin();
        for(; it != _config_map.end(); ++it)
        {
            memset(szPair, 0, sizeof(szPair));
            snprintf(szPair, sizeof(szPair), "%s=%s\n", it->first.c_str(), it->second.c_str());
            size_t ret = ::fwrite(szPair, ::strlen(szPair), 1, fp);
            if(ret != 1)
            {
                fclose(fp);
                return -1;
            }
        }
        fclose(fp);
        return 0;
    }

    void parseLine(char* line)
    {
        char* p = strchr(line, '=');
        // 不是键值对结构，就直接返回吧
        if(p == nullptr)
            return;

        // p 位置本来是 '=' 的位置 将其替换为 '\0' 
        // 这样就把原来的字符串分割为两个
        // line 标记 key 字符串
        // p+1  标记 value 字符串
        *p = '\0';

        // 调用 trimSpace 函数处理空格 使拿到的是一个完整的，不带空格的 key
        char* key = trimSpace(line);
        // 调用 trimSpace 函数 处理空格
        char* value = trimSpace(p+1);
        if(key && value)
        {
            // 当 key 和 value 同时存在的时候，才插入到 map 中
            _config_map.insert(std::make_pair(key, value));
        }
    }

    char* trimSpace(char* name)
    {
        // remove starting space or tab 
        // 删除起始位置的 空格 和 tab 键
        char* startPos = name;
        while((*startPos == ' ') || (*startPos) == '\t')
        {
            ++startPos;
        }

        // remove ending space or tab
        // 删除末尾位置的 空格 和 tab 键
        char* endPos = name + ::strlen(name) - 1;
        while((*endPos == ' ') || (*endPos == '\t'))
        {
            *endPos = '\0';
            --endPos;
        }

        int len = (int)(endPos-startPos) + 1;
        if(len <= 0)
        {
            return nullptr;
        }

        return startPos;
    }
private:
    bool                               _isReadOk;
    std::map<std::string, std::string> _config_map;
    std::string                        _config_file;
};  
