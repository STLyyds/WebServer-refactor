#ifndef LOGFILE_H
#define LOGFILE_H

#pragma once
#include <memory>
#include <string>
#include "FileUtil.h"
#include "MutexLock.h"
#include "noncopyable.h"

class LogFile : noncopyable
{
public:
    // 每被append flushEveryN次，flush一下，会往文件写，只不过文件也带有缓冲区
    LogFile(const std::string& basename, int flushEveryN = 1024);
    ~LogFile();

    void append(const char* logLine, int len);
    void flush();
    bool rollFile();

private:
    void append_unlocked(const char* logLine, int len);

    const std::string m_basename;
    const int m_flushEveryN;

    int m_count;
    std::unique_ptr<MutexLock> m_mutex;
    std::unique_ptr<AppendFile> m_file;
};

#endif