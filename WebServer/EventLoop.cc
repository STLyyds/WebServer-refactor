#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>
#include "Util.h"
#include "base/Logging.h"

using namespace std;

__thread EventLoop* t_loopInThisThread = 0;

// 专门用于事件通知的文件描述符，创建出句柄后，可以对其读、写、监听、关闭
int createEventfd() 
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
    : m_looping(false), 
      m_poller(new Epoll()), 
      m_wakeupFd(createEventfd()), 
      m_quit(false), 
      m_eventHandling(false), 
      m_callingPendingFunctors(false), 
      m_threadId(CurrentThread::tid()), 
      m_pwakeupChannel(new Channel(this, m_wakeupFd))
{
    if (t_loopInThisThread)
    {
        // LOG << "Another EventLoop " << t_loopInThisThread << " exists in this
        // thread " << m_threadId;
    }
    else 
    {
        t_loopInThisThread = this;
    }
    // m_pwakeupChannel->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    m_pwakeupChannel->setEvents(EPOLLIN | EPOLLET);
    m_pwakeupChannel->setReadHandler(bind(&EventLoop::handleRead, this));
    m_pwakeupChannel->setConnHandler(bind(&EventLoop::handleConn, this));
    m_poller->epoll_add(m_pwakeupChannel, 0);
}

void EventLoop::handleConn()
{
    // m_poller->epoll_mod(m_wakeupFd,m_pwakeupChannel, (EPOLLIN | EPOLLET |
    // EPOLLONESHOT), 0);
    updatePoller(m_pwakeupChannel, 0);
}

EventLoop::~EventLoop() 
{
    close(m_wakeupFd);
    t_loopInThisThread = NULL;
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = writen(m_wakeupFd, (char*)(&one), sizeof(one));
    if (n != sizeof(one)) 
    {
        LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = readn(m_wakeupFd, &one, sizeof(one));
    if (n!=sizeof(one)) 
    {
        LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
    m_pwakeupChannel->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::runInLoop(Functor&& cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else 
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor&& cb)
{
    {
        MutexLockGuard lock(m_mutex);
        m_pendingFunctors.emplace_back(std::move(cb));
    }

    if (!isInLoopThread() || m_callingPendingFunctors) wakeup();
}

void EventLoop::loop()
{
    assert(!m_looping);
    assert(isInLoopThread());
    m_looping = true;
    m_quit = false;
    std::vector<SP_Channel> ret;
    while(!m_quit)
    {
        ret.clear();
        ret = m_poller->poll();
        m_eventHandling = true;
        for (auto& it : ret) it->handleEvents();
        m_eventHandling = false;
        doPendingFunctors();
        m_poller->handleExpired();
    }
    m_looping = false;
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    m_callingPendingFunctors = true;

    {
        MutexLockGuard lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }

    for(size_t i = 0; i < functors.size(); ++i) functors[i]();
    m_callingPendingFunctors = false;
}

void EventLoop::quit()
{
    m_quit = true;
    if (!isInLoopThread()) wakeup();
}