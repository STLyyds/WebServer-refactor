#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, int numsThreads)
    : m_baseLoop(baseLoop), m_started(false), m_numThreads(numsThreads), m_next(0)
{
    if (m_numThreads <= 0) 
    {
        LOG << "m_numThreads <= 0";
        abort();
    }
}

void EventLoopThreadPool::start()
{
    m_baseLoop->assertInLoopThread();
    m_started = true;
    for (int i = 0; i < m_numThreads; ++i)
    {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        m_threads.push_back(t);
        m_loops.push_back(t->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    m_baseLoop->assertInLoopThread();
    assert(m_started);
    EventLoop* loop = m_baseLoop;
    if (!m_loops.empty())
    {
        loop = m_loops[m_next];
        m_next = (m_next + 1) % m_numThreads;
    }
    return loop;
}