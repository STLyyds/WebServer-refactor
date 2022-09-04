#ifndef THREAD_H
#define THREAD_H

#pragma once
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <string>
#include "CountDownLatch.h"
#include "noncopyable.h"

class Thread : noncopyable
{
public:
    typedef std::function<void()> ThreadFunc;   // 回调函数

    explicit Thread(const ThreadFunc&, const std::string& name = std::string());
    ~Thread();

    void start();
    int join();
    bool started() const { return m_started; }

    pid_t tid() const { return m_tid; }
    const std::string& name() const { return m_name; }

private:
    void setDefaultName();
    bool m_started;
    bool m_joined;
    pthread_t m_pthreadId;
    pid_t m_tid;
    ThreadFunc m_func;
    std::string m_name;
    CountDownLatch m_latch;
};


#endif