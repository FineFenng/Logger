//
// Created by finefenng on 2017/12/2.
//

#include "LogUtility.h"


#include <sys/stat.h>
#include <sys/time.h>

#include <fcntl.h>

#include <cstring>


static int kMicroSecondsPerSecond = 1000 * 1000;

std::thread::id LogUtility::GetCurrentThreadId()
{
    return std::this_thread::get_id();
}

std::string LogUtility::FormatTimeStampToString()
{
    timeval time;
    ::bzero(&time, sizeof(timeval));
    ::gettimeofday(&time, nullptr);

    int64_t microSecondsSinceEpoch = time.tv_sec * kMicroSecondsPerSecond + time.tv_usec;

    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / kMicroSecondsPerSecond);
    int microSeconds = static_cast<int>(microSecondsSinceEpoch % kMicroSecondsPerSecond);

    tm tm_time;
    ::gmtime_r(&seconds, &tm_time);


    char temp[32] = {0};

    int len = snprintf(temp, sizeof(temp), "%4d%02d%02d %02d:%02d:%02d",
                       tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                       tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);

    snprintf(temp + len, sizeof(temp) - len, "%06dZ", microSeconds);
    return std::string(temp);
}

LogUtility::ReadSmallFile::ReadSmallFile(std::string& filename)
        : impl_(std::make_shared<Impl>(::open(filename.c_str(), O_RDONLY | O_CLOEXEC)), 0)
{
    impl_->buf_[0] = '\0';
    if (impl_->fd_ < 0) {
        impl_->err_ = errno;
    }
}

LogUtility::ReadSmallFile::~ReadSmallFile()
{
    if (impl_->fd_ > 0) {
        ::close(impl_->fd_);
    }
}

int LogUtility::ReadSmallFile::readToString(int maxSize, std::string& content, int64_t* fileSize, int64_t* modifyTime,
                                            int64_t* createTime)
{
    int err = impl_->err_;

    if (impl_->fd_) {
        content.clear();

        if (fileSize) {
            stat statBuff;
            ::bzero(&statBuff, sizeof(statBuff));
            if (::fstat(impl_->fd_, &statBuff) == 0) {
                if (S_ISREG(statBuff.st_mode)) {
                    *fileSize = statBuff.st_size;
                    auto minSize = std::min(static_cast<int64_t >(maxSize), *fileSize);
                    content.reserve(static_cast<unsigned long>(minSize));
                } else if (S_ISDIR(statBuff.st_mode)) {
                    err = EISDIR;
                } else {

                }

                if (modifyTime) {
                    *modifyTime = statBuff.st_mtimespec.tv_sec;
                }

                if (createTime) {
                    *createTime = statBuff.st_ctimespec.tv_sec;
                }
            }
        } else {
            err = errno;
        }
    }

    while (content.size() < static_cast<size_t >(maxSize)) {
        size_t buffSize = sizeof(impl_->buf_);
        size_t toRead = std::min(static_cast<size_t >(buffSize), buffSize);
        ssize_t n = ::read(impl_->fd_, impl_->buf_, toRead);

        if (n > 0) {
            content.append(impl_->buf_, n);
        } else {
            if (n < 0) {
                err = errno;
            }
            break;
        }
    }

    return err;
}


int LogUtility::ReadSmallFile::readToOwnBuffer(int* size)
{
    int err = impl_->err_;

    if (impl_->fd_ > 0) {
        ssize_t n = ::pread(impl_->fd_, impl_->buf_, sizeof(impl_->buf_) - 1, 0);
        if (n >= 0) {
            if (size) {
                *size = static_cast<int>(n);
            }
            impl_->buf_[n] = '\0';
        } else {
            err = errno;
        }
    }
    return err;
}

const char* LogUtility::ReadSmallFile::buffer() const
{
    return impl_->buf_;
}


class LogUtility::ReadSmallFile::Impl
{
public:
    Impl(int fd, int err)
            : fd_(fd), err_(err), buf_{0}
    {}


public:
    int fd_;
    int err_;
    char buf_[kBufferSize];
};


LogUtility::AppendFile::AppendFile(const std::string& filename)
        : impl_(std::make_shared<Impl>(::fopen(filename.c_str(), "ae")))
{
    ::setbuffer(impl_->fp_, impl_->buffer_, sizeof(impl_->buffer_));
}

LogUtility::AppendFile::~AppendFile()
{
    ::fclose(impl_->fp_);
}

void LogUtility::AppendFile::append(const char* logLine, size_t len)
{
    size_t n = impl_->write(logLine, len);
    size_t remain = len - n;
    while (remain > 0) {
        size_t x = impl_->write(logLine + n, remain);
        if (x == 0) {
            int err = ::ferror(impl_->fp_);
            if (err) {
                char temp[32] = {0};
                strerror_r(err, temp, sizeof(temp));
                fprintf(stderr, "AppendFile::append() failed %s\n", temp);
            }
            break;
        }
        n += x;
        remain = len - n;
    }
    impl_->writtenBytes_ += len;
}

void LogUtility::AppendFile::flush()
{
    ::fflush(impl_->fp_);
}

size_t LogUtility::AppendFile::writtenBytes() const
{
    return impl_->writtenBytes_;
}


class LogUtility::AppendFile::Impl
{
public:
    explicit Impl(FILE* fp)
            : fp_(fp), buffer_{0}, writtenBytes_(0)
    {}

    size_t write(const char* logLine, size_t len)
    {
        ::fwrite(logLine, 1, len, fp_);
    }


public:

    FILE* fp_;
    char buffer_[64 * 1024];
    size_t writtenBytes_;

};


