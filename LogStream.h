//
// Created by finefenng on 2017/12/1.
//

#ifndef LOGGER_LOGSTREAM_H
#define LOGGER_LOGSTREAM_H


#include <cstddef>
#include <cstring>

#include <memory>
#include <string>
#include <thread>
#include "boost/noncopyable.hpp"



namespace detail
{
    const int kSmallBuffer = 4000;
    const int kLargeBuffer = 4000 * 1000;

    template <int SiZE>
    class FixedBuffer :boost::noncopyable
    {
    public:
        FixedBuffer()
            : data_{0}, cur_(data_)
        { }


        void append(const char* buf, size_t len)
        {
            if (avail() > static_cast<int>(len)) {
                memcpy(cur_, buf, len);
                cur_ += len;
            }

        }

        const char* data() const { return data_; }
        char* current() const { return cur_;}
        int length() const { return static_cast<int>(cur_ - data_); }
        void add(size_t len) { cur_ += len; }
        void reset() { cur_ = data_; }
        void bzero() { ::bzero(data_, sizeof(data_)); }
        int avail() const { return static_cast<int>(end() - cur_); }


    private:

        const char* end() const { return data_ + sizeof(data_); }

        char data_[SiZE];
        char* cur_;
    };
}

class LogStream : boost::noncopyable
{
public:
    typedef LogStream self;
    typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

    LogStream();


    self& operator<<(bool val);
    self& operator<<(unsigned short val);
    self& operator<<(short val);
    self& operator<<(unsigned int val);
    self& operator<<(int val);
    self& operator<<(unsigned long val);
    self& operator<<(long val);
    self& operator<<(unsigned long long val);
    self& operator<<(long long val);

    self& operator<<(float val);
    self& operator<<(double val);

    self& operator<<(char ch);
    self& operator<<(const char* str);
    self& operator<<(const std::string& str);

    self& operator<<(const std::thread::id & id);

    void append(const char* data, int len);
    const Buffer& buffer() const;
    void resetBuffer();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};


#endif //LOGGER_LOGSTREAM_H
