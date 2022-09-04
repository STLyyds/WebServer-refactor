#include "AsyncLogging.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <functional>
#include "LogFile.h"

AsyncLogging::AsyncLogging(std::string logFileName, int flushInterval)
    : m_flushInterval(flushInterval), 
      m_running(false), 
      m_basename(logFileName),
      m_thread(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
      m_mutex(), 
      m_cond(m_mutex),
      m_currentBuffer(new Buffer), 
      m_nextBuffer(new Buffer), 
      m_buffers(), 
      m_latch(1)
{
    assert(logFileName.size() > 1);
    m_currentBuffer->bzero();
    m_nextBuffer->bzero();
    m_buffers.reserve(16);
}

void AsyncLogging::append(const char* logLine, int len) 
{
    MutexLockGuard lock(m_mutex);
    if(m_currentBuffer->avail() > len)
    {
        m_currentBuffer->append(logLine,len);
    }
    else
    {
        m_buffers.push_back(m_currentBuffer);
        m_currentBuffer.reset();
        if(m_nextBuffer)
        {
            m_currentBuffer = m_nextBuffer;
        }
        else 
        {
            m_currentBuffer.reset(new Buffer);
        }
        m_currentBuffer->append(logLine, len);
        // 有已满的buffer，需要唤醒后端线程将已满缓冲数据写入磁盘
        m_cond.notify();
    }
}

void AsyncLogging::threadFunc()
{
    assert(m_running == true);
    m_latch.countDown();
    LogFile output(m_basename);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while(m_running) 
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            MutexLockGuard lock(m_mutex);
            if(m_buffers.empty()) // unusual usage !!!
            {
                m_cond.waitForSeconds(m_flushInterval);
            }
            m_buffers.push_back(m_currentBuffer);
            m_currentBuffer.reset();

            m_currentBuffer = std::move(newBuffer1);
            buffersToWrite.swap(m_buffers);
            if (!m_nextBuffer)
            {
                m_nextBuffer = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        if(buffersToWrite.size() > 25) // 一个日志buffer大小为4MB，25个则是100MB，超过25个即日志堆积超过100MB，丢弃多余缓冲，只保留2个去move给空闲缓冲
        {
            // 将丢弃多余缓冲记录
            // char buf[256];
            // snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger
            // buffers\n",
            //          Timestamp::now().toFormattedString().c_str(),
            //          buffersToWrite.size()-2);
            // fputs(buf, stderr);
            // output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }

        for(size_t i = 0; i < buffersToWrite.size(); i++)
        {
            output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
        }

        if(buffersToWrite.size() > 2)
        {
            // 丢弃多余缓冲
            buffersToWrite.resize(2);
        }

        if(!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if(!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}