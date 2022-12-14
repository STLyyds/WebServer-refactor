#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H

#pragma once
#include <functional>
#include <string>
#include <vector>
#include "CountDownLatch.h"
#include "LogStream.h"
#include "MutexLock.h"
#include "Thread.h"
#include "noncopyable.h"

class AsyncLogging : noncopyable
{
public:
    AsyncLogging(const std::string basename, int flushInterval = 2);
    ~AsyncLogging()
    {
        if (m_running) stop();
    }

    void append(const char* logLine, int len);

    void start()
    {
        m_running = true;
        m_thread.start();
        m_latch.wait();
    }

    void stop()
    {
        m_running = false;
        m_cond.notify();
        m_thread.join();
    }

private:
    void threadFunc();
    
    typedef FixedBuffer<kLargeBuffer> Buffer; // 存放一段时间内的日志，4KB
    typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
    typedef std::shared_ptr<Buffer> BufferPtr;

    const int m_flushInterval;
    bool m_running;
    std::string m_basename;
    Thread m_thread;
    MutexLock m_mutex;
    Condition m_cond;
    BufferPtr m_currentBuffer;
    BufferPtr m_nextBuffer;
    BufferVector m_buffers;
    CountDownLatch m_latch;
};


#endif