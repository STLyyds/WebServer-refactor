#include "LogFile.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "FileUtil.h"

using namespace std;

LogFile::LogFile(const string& basename, int flushEveryN)
    : m_basename(basename),
      m_flushEveryN(flushEveryN), 
      m_count(0), 
      m_mutex(new MutexLock)
{
    // assert(basename.find('/') >= 0);
    m_file.reset(new AppendFile(basename));
}

LogFile::~LogFile() {}

void LogFile::append(const char* logLine, int len) 
{
    MutexLockGuard lock(*m_mutex);
    append_unlocked(logLine, len);
}

void LogFile::flush()
{
    MutexLockGuard lock(*m_mutex);
    m_file->flush();
}

void LogFile::append_unlocked(const char* logLine, int len) 
{
    m_file->append(logLine, len);
    ++m_count;
    if(m_count >= m_flushEveryN) 
    {
        m_count = 0;
        m_file->flush();
    }
}