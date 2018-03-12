//
// Created by finefenng on 2017/12/1.
//

#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H

#include <cstring>
#include <memory>

#include "boost/noncopyable.hpp"


class LogStream;

class Logger : boost::noncopyable
{
public:
    enum Level
    {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };

    typedef Logger::Level LogLevel;

    class SourceFile
    {
    public:
        template<int N>
        inline SourceFile(const char(& arr)[N])
        {
            data_ = arr;
            size_ = N;

            const char* slash = strrchr(data_, '/');

            if (slash) {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char* fileName)
                : data_(fileName)
        {
            const char* slash = strrchr(fileName, '/');
            if (slash) {
                data_ = slash + 1;
            }

            size_ = static_cast<int>(strlen(data_));
        }

    private:
        const char* data_;
        int size_;
    };

    Logger(SourceFile sourceFile, int len);

    Logger(SourceFile sourceFile, int len, Logger::Level level);

    Logger(SourceFile sourceFile, int len, Logger::Level level, const char* func);

    Logger(SourceFile sourceFile, int len, bool toAbort);

    ~Logger();

    LogStream& stream();

    static LogLevel logLevel();

    static void setLogLevel(Logger::Level level);

    typedef void(* OutPutFunc)(const char* message, int len);

    typedef void(* FlushFunc)();

    static void setOutPutFunc(OutPutFunc);

    static void setFlushFunc(FlushFunc);

private:
    class Impl;

    std::shared_ptr<Impl> impl_;


};

extern Logger::Level g_logLevel;


#define LOG_TRACE if (Logger::loglevel() <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()


#define LOG_DEBUG if (Logger::loglevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()

#define LOG_INFO if (Logger::loglevel() <= Logger::INFO) \
  Logger(__FILE__, __LINE__).stream()

#define LOG_WARN Logger(__FIFE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FIFE__, __LINE__, Logger:ERROR).stream()
#define LOG_FATAL Logger(__FIFE__, __LINE__, Logger::FATAL).stream()


#endif //LOGGER_LOGGER_H
