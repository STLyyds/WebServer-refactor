#include "Thread.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include "CurrentThread.h"
#include <iostream>

using namespace std;

namespace CurrentThread 
{
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char* t_threadName = "default";
}

// 通过linux下的系统调用获取线程id
pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

void CurrentThread::cacheTid()
{
    if(t_cachedTid == 0) 
    {
        t_cachedTid = gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString), "%5d ", t_cachedTid);
    }
}

// 为了在线程中保留name，tid这些数据
struct ThreadData 
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    string name_;
    pid_t* tid_;
    CountDownLatch* latch_;

    ThreadData(const ThreadFunc& func, const string& name, pid_t* tid, CountDownLatch* latch) 
    : func_(func), name_(name), tid_(tid), latch_(latch) {}

    void runInThread()
    {
        *tid_ = CurrentThread::tid();
        tid_ = NULL;
        latch_->countDown();
        latch_ = NULL;

        CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
        prctl(PR_SET_NAME, CurrentThread::t_threadName);

        func_();
        CurrentThread::t_threadName = "finished";
    }
};

void* startThread(void* obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

Thread::Thread(const ThreadFunc& func, const string& n)
: m_started(false), m_joined(false), m_pthreadId(0), m_tid(0), m_func(func), 
m_name(n), m_latch(1)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(m_started && !m_joined) pthread_detach(m_pthreadId);
}

void Thread::setDefaultName()
{
    if(m_name.empty()) 
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread");
        m_name = buf;
    }
}

void Thread::start() 
{
    assert(!m_started);
    m_started = true;
    ThreadData* data = new ThreadData(m_func, m_name, &m_tid, &m_latch);
    if (pthread_create(&m_pthreadId, NULL, &startThread, data))
    {
        m_started = false;
        delete data;
    }
    else 
    {
        m_latch.wait();
        assert(m_tid > 0);
    }
}

int Thread::join() 
{
    assert(m_started);
    assert(!m_joined);
    m_joined = true;
    return pthread_join(m_pthreadId, NULL);
}