#ifndef FILEUTIL_H
#define FILEUTIL_H

#pragma once

#include <string>
#include "noncopyable.h"

class AppendFile : noncopyable
{
public:
    explicit AppendFile(std::string fileName);
    ~AppendFile();

    void append(const char* logLine, const size_t len);
    void flush();
private:
    size_t write(const char* logLine, size_t len);
    FILE* m_fp;
    char m_buffer[64 * 1024];
};

#endif