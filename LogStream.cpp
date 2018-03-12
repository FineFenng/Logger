//
// Created by finefenng on 2017/12/1.
//

#include "LogStream.h"

#include <sstream>

static size_t kMaxNumericSize = 32;
static char digits[] = {'9', '8', '7', '6', '5', '4', '3', '2', '1',
                        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

static char* zero = digits + 9;

template<typename T>
size_t convertNumToString(char* buf, T t)
{
    T i = t;
    char* p = buf;

    do {
        auto lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (t < 0) {
        *p++ = '-';
    }

    *p = '\0';

    std::reverse(buf, p);
    return p - buf;
}

LogStream::LogStream()
: impl_(new Impl())
{}


LogStream::self& LogStream::operator<<(bool val)
{
    char temp = val ? '1' : '0';
    impl_->buffer_.append(&temp, 1);
    return *this;
}

LogStream::self& LogStream::operator<<(unsigned short val)
{
    return operator<<(static_cast<unsigned int>(val));
}

LogStream::self& LogStream::operator<<(short val)
{
    return operator<<(static_cast<int>(val));
}

LogStream::self& LogStream::operator<<(unsigned int val)
{
    impl_->formatInteger(val);
    return *this;

}

LogStream::self& LogStream::operator<<(int val)
{
    impl_->formatInteger(val);
    return *this;
}

LogStream::self& LogStream::operator<<(unsigned long val)
{
    impl_->formatInteger(val);
    return *this;
}

LogStream::self& LogStream::operator<<(long val)
{
    impl_->formatInteger(val);
    return *this;
}

LogStream::self& LogStream::operator<<(unsigned long long val)
{
    impl_->formatInteger(val);
    return *this;
}

LogStream::self& LogStream::operator<<(long long val)
{
    impl_->formatInteger(val);
    return *this;
}

LogStream::self& LogStream::operator<<(float val)
{
    return operator<<(static_cast<double>(val));
}

LogStream::self& LogStream::operator<<(double val)
{
    if (impl_->buffer_.avail() >= kMaxNumericSize) {
        int len = snprintf(impl_->buffer_.current(), kMaxNumericSize, "%.12g", val);
        impl_->buffer_.add(static_cast<size_t>(len));
    }
    return *this;
}

LogStream::self& LogStream::operator<<(char ch)
{
    impl_->buffer_.append(&ch, 1);
    return *this;
}

LogStream::self& LogStream::operator<<(const char* str)
{
    size_t len = strlen(str);
    impl_->buffer_.append(str, len);
    return *this;
}

LogStream::self& LogStream::operator<<(const std::string& str)
{
    return operator<<(str.c_str());
}


void LogStream::append(const char* str, int len)
{
    impl_->buffer_.append(str, static_cast<size_t>(len));
}


const LogStream::Buffer& LogStream::buffer() const
{
    return impl_->buffer_;
}

void LogStream::resetBuffer()
{
    impl_->buffer_.reset();
    impl_->buffer_.bzero();
}


class LogStream::Impl
{
public:
    template<typename T>
    void formatInteger(T v)
    {
        if (buffer_.avail() >= kMaxNumericSize) {
            size_t len = convertNumToString(buffer_.current(), v);
            buffer_.add(len);
        }
    }

    Buffer buffer_;
};


LogStream::self& LogStream::operator<<(const std::thread::id& id)
{

    std::ostringstream output_stream;
    output_stream << id;

    operator<<(output_stream.str());

    return *this;
}


