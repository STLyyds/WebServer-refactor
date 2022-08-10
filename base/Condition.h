#ifndef CONDITION_H
#define CONDITION_H

#pragma once

#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <cstdint>

#include "noncopyable.h"
#include "MutexLock.h"

class Condition : noncopyable
{
public:
    explicit Condition(MutexLock& _mutex) : 
    m_mutex(_mutex)
    {
        pthread_cond_init(&m_cond, NULL);
    }
    ~Condition() { pthread_cond_destroy(&m_cond); };

    void wait() { pthread_cond_wait(&m_cond, m_mutex.get()); }

    void notify() { pthread_cond_signal(&m_cond); }
    void notifyAll() { pthread_cond_broadcast(&m_cond); }

    bool waitForSeconds(int seconds) 
    {
        struct timespec asbtime;
        clock_gettime(CLOCK_REALTIME, &asbtime);
        asbtime.tv_sec += static_cast<time_t>(seconds);
        return ETIMEDOUT == pthread_cond_timedwait(&m_cond, m_mutex.get(), &asbtime);
    }

private:
    MutexLock &m_mutex;
    pthread_cond_t m_cond;
};

#endif