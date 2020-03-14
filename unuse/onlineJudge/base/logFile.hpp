#pragma once

#include <iostream>
#include <cstdio>
#include <cassert>

#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <memory>

#include "./fileUtil.hpp"

namespace FileUtil
{
    class AppendFile;
}

class LogFile
{
public:
    LogFile(const std::string& baseName,
            size_t rollSize,
            bool threadSafe = true,
            int flushInterval = 3,
            int checkEveryN = 1024)
        : _baseName(baseName)
        , _rollSize(rollSize)
        , _flushInterval(flushInterval)
        , _checkEveryN(checkEveryN)
        , _count(0)
        , _mutex(threadSafe ? new std::mutex : nullptr)
        , _startOfPeriod(0)
        , _lastRoll(0)
        , _lastFlush(0)
    {}

    ~LogFile()
    {}

    void append(const char* logLine, int len)
    {
        if(_mutex)
        {
            std::unique_lock<std::mutex> lock(*_mutex);
            append_unlocked(logLine, len);
        }
        else
        {
            append_unlocked(logLine, len);
        }
    }

    void flush()
    {
        if(_mutex)
        {
            std::unique_lock<std::mutex> lock(*_mutex);
            _file->flush();
        }
    }

    bool rollFile()
    {
        time_t now = 0;
        std::string fileName = getLogFileName(_baseName, &now);
        time_t start = now / _kRollPerSeconds * _kRollPerSeconds;

        if(now > _lastRoll)
        {
            _lastRoll = now;
            _lastFlush = now;
            _startOfPeriod = start;
            _file.reset(new FileUtil::AppendFile(fileName));
            return true;
        }
        return false;
    }

private:
    void append_unlocked(const char* logLine, 
                         int len)
    {
        _file->append(logLine, len);

        if(_file->writtenBytes() > _rollSize)
        {
            rollFile();
        }
        else
        {
            ++_count;
            if(_count >= _checkEveryN)
            {
                _count = 0;
                time_t now = ::time(nullptr);
                time_t thisPeriod = now / _kRollPerSeconds * _kRollPerSeconds;
                if(thisPeriod != _startOfPeriod)
                {
                    rollFile();
                }
                else if(now - _lastFlush > _flushInterval)
                {
                    _lastFlush = now;
                    _file->flush();
                }
            }
        }
    }

    static std::string getLogFileName(const std::string& baseName,
                                      time_t * now)
    {
        std::string fileName;
        fileName.reserve(baseName.size() + 64);
        fileName = baseName;

        char timeBuf[32] = {0};
        struct tm st_tm;
        // 因为这个是私有成员函数，我们是可以保证now有效
        *now = time(nullptr);
        localtime_r(now, &st_tm);
        strftime(timeBuf, sizeof(timeBuf), ".%Y%m%d-%H%M%S", &st_tm);

        fileName += ".log";

        return fileName;
    }

private:
    const std::string _baseName;
    const size_t      _rollSize;
    const int         _flushInterval;
    const int         _checkEveryN;

    int               _count;

    std::shared_ptr<std::mutex> _mutex;
    time_t            _startOfPeriod;
    time_t            _lastRoll;
    time_t            _lastFlush;
    std::shared_ptr<FileUtil::AppendFile> _file;

    // 一天的时间？
    // 24h * 60min * 60s
    const static int _kRollPerSeconds = 60 * 60 * 24;
};
