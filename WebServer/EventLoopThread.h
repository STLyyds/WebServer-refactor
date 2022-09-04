#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H
#pragma once
#include "EventLoop.h"
#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/Thread.h"
#include "base/noncopyable.h"

class EventLoopThread : noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();
    EventLoop* m_loop;
    bool m_exiting;
    Thread m_thread;
    MutexLock m_mutex;
    Condition m_cond;
};

#endif