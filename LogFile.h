//
// Created by finefenng on 2017/12/2.
//

#ifndef LOGGER_LOGFILE_H
#define LOGGER_LOGFILE_H

#include <memory>
#include <string>

#include "boost/noncopyable.hpp"


class LogFile : boost::noncopyable
{
public:
    LogFile(const std::string& basename, size_t rollSize, int flushInterval = 3, int checkEveryN = 1024);

    ~LogFile();

    void append(const char* logLine, int len);

    void flush();

    bool rollFile();

private:

    class Impl;

    std::shared_ptr<Impl> impl_;


};


#endif //LOGGER_LOGFILE_H
