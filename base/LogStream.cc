#include "LogStream.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <limits>

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

// 数值到字符串的转换
template <typename T>
size_t convert(char buf[], T value)
{
    T i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

template <typename T>
void LogStream::formatInteger(T v)
{
    // buffer容不下kMaxNumericSize个字符的话会被直接丢弃
    if (m_buffer.avail() >= kMaxNumericSize)
    {
        size_t len = convert(m_buffer.current(), v);
        m_buffer.add(len);
    }
}

LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v) 
{
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v) 
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v) 
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long v) 
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v) 
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v) 
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) 
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(double v)
{
    if (m_buffer.avail() >= kMaxNumericSize)
    {
        // %g 自动选择%f, %e格式, 并且不输出无意义0. %.12g 最多保留12位小数
        int len = snprintf(m_buffer.current(), kMaxNumericSize, "%.12g", v);
        m_buffer.add(len);
    }
    return *this;
}

LogStream& LogStream::operator<<(long double v)
{
    if (m_buffer.avail() >= kMaxNumericSize)
    {
        int len = snprintf(m_buffer.current(), kMaxNumericSize, "%.12Lg", v);
        m_buffer.add(len);
    }
    return *this;
}