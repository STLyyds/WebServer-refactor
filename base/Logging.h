#ifndef LOGGING_H
#define LOGGING_H

#pragma once

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "LogStream.h"

class AsyncLogging;

class Logger
{
public:
    Logger(const char* fileNmae, int line);
    ~Logger();

    LogStream& stream() { return m_impl.m_stream; }

    static void setLogFileName(std::string fileName) { m_logFileName = fileName; }
    static std::string getLogFileName() { return m_logFileName; }

private:
    class Impl
    {
    public:
        Impl(const char* fileName, int line);
        void formatTime();

        LogStream m_stream;
        int m_line;
        std::string m_basename;
    };
    Impl m_impl;
    static std::string m_logFileName;
};

#define LOG Logger(__FILE__, __LINE__).stream()
/*
ANSI C标准中有几个标准预定义宏（也是常用的）：

__LINE__：在源代码中插入当前源代码行号；

__FILE__：在源文件中插入当前源文件名；
*/

#endif