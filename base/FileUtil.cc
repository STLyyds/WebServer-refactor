#include "FileUtil.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

AppendFile::AppendFile(string fileName) : m_fp(fopen(fileName.c_str(), "ae"))
{
    // 用户提供缓冲区
    setbuffer(m_fp, m_buffer, sizeof(m_buffer));
}

AppendFile::~AppendFile() { fclose(m_fp); }

void AppendFile::append(const char* logLine, const size_t len)
{
    size_t n = this->write(logLine, len);
    size_t remain = len - n;
    while(remain > 0)
    {
        size_t x = this->write(logLine + n, remain);
        if(x == 0) 
        {
            int err = ferror(m_fp);
            if (err) fprintf(stderr, "AppendFile::append() failed !\n");
            break;
        }
        n += x;
        remain = len - n;
    }
}

void AppendFile::flush() { fflush(m_fp); }

size_t AppendFile::write(const char* logLine, size_t len) 
{
    // fwrite无锁版，比正常的快，但是线程不安全
    return fwrite_unlocked(logLine, 1, len, m_fp);
}