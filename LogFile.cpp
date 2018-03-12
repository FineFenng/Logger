//
// Created by finefenng on 2017/12/2.
//

#include "LogFile.h"

#include <chrono>
#include <mutex>

#include "LogUtility.h"


typedef std::chrono::seconds Seconds;


LogFile::LogFile(const std::string& basename, size_t rollSize, int flushInterval, int checkEveryN)
        : impl_(std::make_shared<Impl>(basename, rollSize, flushInterval, checkEveryN))
{
    rollFile();
}

LogFile::~LogFile()
{}

void LogFile::append(const char* logLine, int len)
{
    impl_->append_unlocked(logLine, len);
}

void LogFile::flush()
{
    impl_->file_->flush();
}

bool LogFile::rollFile()
{
    Seconds now(0);

    std::string filename = impl_->getLogFileName(impl_->basename_, now);

    /* TODO */
}

class LogFile::Impl
{
public:

    Impl(std::string& basename, size_t rollSize, int flushInterval, int checkEveryN)
            : basename_(basename), rollSize_(rollSize), flushInterval_(flushInterval), checkEveryN_(checkEveryN),
              count_(0), startOfPeriod_(0), lastRoll_(0), lastFlush_(0), file_()
    {}

    void append_unlocked(const char* logLine, int len)
    {

    }

public:

    static std::string getLogFileName(const std::string& basename, Seconds& now)
    {
        std::string filename;
        filename.reserve(basename.size() + 6);
        filename = basename;

        char timeBuff[32] = {0};
        tm tm_time;

        using namespace std::chrono;

        time_point<system_clock, Seconds> tp = system_clock::now();


        /* todo */
    }

public:

    const std::string basename_;
    const size_t rollSize_;
    const int flushInterval_;
    const int checkEveryN_;

    int count_;

    Seconds startOfPeriod_;
    Seconds lastRoll_;
    Seconds lastFlush_;

    std::unique_ptr<LogUtility::AppendFile> file_;

public:
    const static int kRollPerSeconds_ = 60 * 60 * 24;
};


