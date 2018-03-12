//
// Created by finefenng on 2017/12/2.
//

#ifndef LOGGER_LOGUTILITY_H
#define LOGGER_LOGUTILITY_H

#include <string>
#include <thread>

#include "boost/noncopyable.hpp"

namespace LogUtility {
    std::thread::id GetCurrentThreadId();

    std::string FormatTimeStampToString();


    class ReadSmallFile : boost::noncopyable
    {
    public:
        explicit ReadSmallFile(std::string& filename);

        ~ReadSmallFile();

        int readToString(int maxSize, std::string& content, int64_t* fileSize, int64_t* modifyTime, int64_t* createTime);

        int readToOwnBuffer(int* size);

        const char* buffer() const;

        static const int kBufferSize = 60 * 1024;

    private:
        class Impl;

        std::shared_ptr<Impl> impl_;
    };


    class AppendFile : boost::noncopyable
    {
    public:
        explicit AppendFile(const std::string& filename);

        ~AppendFile();

        void append(const char* logLine, size_t len);

        void flush();

        size_t writtenBytes() const;

    private:

        class Impl;

        std::shared_ptr<Impl> impl_;

    };
}


#endif //LOGGER_LOGUTILITY_H
