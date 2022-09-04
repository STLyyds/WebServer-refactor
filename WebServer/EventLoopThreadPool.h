#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H
#pragma once
#include <memory>
#include <vector>
#include "EventLoopThread.h"
#include "base/Logging.h"
#include "base/noncopyable.h"

class EventLoopThreadPool : noncopyable
{
public:
    EventLoopThreadPool(EventLoop* baseLoop, int numsThreads);
    ~EventLoopThreadPool() { LOG << "~EventLoopThreadPool()"; }

    void start();

    EventLoop* getNextLoop();

    
private:
    EventLoop* m_baseLoop;
    bool m_started;
    int m_numThreads;
    int m_next;
    std::vector<std::shared_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop*> m_loops;
};

#endif