#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#pragma once
#include <iostream>
#include <functional>
#include <memory>
#include <vector>
#include "Channel.h"
#include "Epoll.h"
#include "Util.h"
#include "base/CurrentThread.h"
#include "base/Logging.h"
#include "base/Thread.h"

using namespace std;

class EventLoop
{
public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void runInLoop(Functor&& cb);
    void queueInLoop(Functor&& cb);
    bool isInLoopThread() const { return m_threadId == CurrentThread::tid(); }
    void assertInLoopThread() { assert(isInLoopThread()); }
    void shutdown(shared_ptr<Channel> channel) { shutDownWR(channel->getFd()); }

    void removeFromPoller(shared_ptr<Channel> channel)
    {
        m_poller->epoll_del(channel);
    }
    void updatePoller(shared_ptr<Channel> channel, int timeout = 0)
    {
        m_poller->epoll_mod(channel, timeout);
    }
    void addToPoller(shared_ptr<Channel> channel, int timeout = 0)
    {
        m_poller->epoll_add(channel, timeout);
    }

private:
    bool m_looping;
    shared_ptr<Epoll> m_poller;
    int m_wakeupFd;
    bool m_quit;
    bool m_eventHandling;
    mutable MutexLock m_mutex;
    std::vector<Functor> m_pendingFunctors;
    bool m_callingPendingFunctors;
    const pid_t m_threadId;
    shared_ptr<Channel> m_pwakeupChannel;

    void wakeup();
    void handleRead();
    void doPendingFunctors();
    void handleConn();
};

#endif