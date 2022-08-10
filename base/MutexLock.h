#ifndef MUTEXLOCK_H
#define MUTEXLOCK_H

#pragma once

#include <pthread.h>
#include <cstdio>
#include "noncopyable.h"

class MutexLock : noncopyable
{
public:
    MutexLock() { pthread_mutex_init(&m_mutex, NULL); }
    ~MutexLock() 
    {
        // 首先进行加锁，作用在于得到锁，再进行
        pthread_mutex_lock(&m_mutex);
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() { pthread_mutex_lock(&m_mutex); }
    void unlock() { pthread_mutex_unlock(&m_mutex); }

    pthread_mutex_t* get() { return &m_mutex; }

private:
    pthread_mutex_t m_mutex;

// 友元类不受访问权限影响
private:
    friend class Condition;
};

class MutexLockGuard : noncopyable
{
public:
    explicit MutexLockGuard(MutexLock& _mutex) :
    m_mutex(_mutex)
    {
        m_mutex.lock();
    }
    ~MutexLockGuard()
    {
        m_mutex.unlock();
    }
private:
    MutexLock& m_mutex;
};


#endif