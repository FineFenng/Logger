//
// Created by finefenng on 2017/12/1.
//

#include "Logger.h"

#include <cstdlib>
#include <cstdio>

#include "LogStream.h"
#include "LogUtility.h"


Logger::Level initLogLevel()
{
    if (::getenv("LOG_TRACE")) {
        return Logger::TRACE;
    } else if (::getenv("LOG_DEBUG")) {
        return Logger::DEBUG;
    } else {
        return Logger::INFO;
    }
}


Logger::Level g_logLevel = initLogLevel();


inline LogStream& operator<<(LogStream& s, const Logger::SourceFile v)
{
    s.append(v.data_, v.size_);
    return s;
}

void defaultOutPut(const char* message, int len)
{
    size_t n = ::fwrite(message, 1, static_cast<size_t>(len), stdout);
}

void defaultFlush()
{
    ::fflush(stdout);
}

Logger::OutPutFunc g_outPutFunc = defaultOutPut;
Logger::FlushFunc g_flushFunc = defaultFlush;

class Logger::Impl
{
public:
    typedef Logger::Level LogLevel;
    typedef std::string TimeStamp;

    Impl(LogLevel level, int saveErrno, const SourceFile& file, int line)
            : stream_(), level_(level), line_(line), basename_(file)
    {
        auto cur_id = LogUtility::GetCurrentThreadId();
        auto timeStampString = LogUtility::FormatTimeStampToString();
        stream_ << cur_id << timeStampString;
    }

    void finish()
    {
        stream_ << " - " << basename_ << ':' << line_ << '\n';
    }

public:
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
    TimeStamp time_;
};


Logger::Logger(Logger::SourceFile sourceFile, int len)
        : impl_(new Impl(INFO, 0, sourceFile, len))
{

}

Logger::Logger(Logger::SourceFile sourceFile, int len, Logger::Level level)
        : impl_(new Impl(level, 0, sourceFile, len))
{
}

Logger::Logger(Logger::SourceFile sourceFile, int len, Logger::Level level, const char* func)
        : impl_(new Impl(level, 0, sourceFile, len))
{
    impl_->stream_ << func << ' ';
}

Logger::Logger(Logger::SourceFile sourceFile, int len, bool toAbort)
        : impl_(new Impl(toAbort ? FATAL : ERROR, 0, sourceFile, len))
{}

Logger::LogLevel Logger::logLevel()
{
    return ERROR;
}

void Logger::setLogLevel(Logger::Level level)
{
    g_logLevel = level;
}

Logger::~Logger()
{
    impl_->finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_outPutFunc(buf.data(), buf.length());
    if (impl_->level_ == FATAL) {
        g_flushFunc();
        abort();
    }
}

LogStream& Logger::stream()
{
    return impl_->stream_;
}

void Logger::setOutPutFunc(Logger::OutPutFunc outPutFunc)
{
    g_outPutFunc = outPutFunc;
}

void Logger::setFlushFunc(Logger::FlushFunc flushFunc)
{
    g_flushFunc = flushFunc;
}




