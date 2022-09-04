#include "Channel.h"
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <queue>
#include "Util.h"
#include "Epoll.h"
#include "EventLoop.h"

using namespace std;

Channel::Channel(EventLoop* loop)
    : m_loop(loop), m_events(0), m_lastEvents(0), m_fd(0) {}

Channel::Channel(EventLoop* loop, int fd)
    : m_loop(loop), m_events(0), m_lastEvents(0), m_fd(fd) {}

Channel::~Channel()
{
    // m_loop->m_poller->epoll_del(fd, m_events);
    // close(m_fd);
}

int Channel::getFd() { return m_fd; }
void Channel::setFd(int fd) { m_fd = fd; }

void Channel::handleRead() 
{
    if(m_readHandler) 
    {
        m_readHandler();
    }
}

void Channel::handleWrite() 
{
    if (m_writeHandler) 
    {
        m_writeHandler();
    }
}

void Channel::handleConn() 
{
    if (m_connHandler) 
    {
        m_connHandler();
    }
}