//
// Created by finefenng on 2017/12/2.
//

#ifndef LOGGER_ASYNCLOGGING_H
#define LOGGER_ASYNCLOGGING_H


#include <memory>
#include <string>


class AsyncLogging
{
public:
    AsyncLogging(const std::string& basename, size_t rollSize, int flushInterval = 3);

    ~AsyncLogging();

    void append(const char* logLine, int len);

    void start();

    void stop();

private:

    class Impl;

    std::shared_ptr<Impl> impl_;
};


#endif //LOGGER_ASYNCLOGGING_H
