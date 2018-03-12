//
// Created by finefenng on 2017/12/2.
//

#include "AsyncLogging.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <vector>

#include "LogFile.h"
#include "LogStream.h"
#include "LogUtility.h"


typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;
typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
typedef BufferVector::value_type BufferPtr;


AsyncLogging::AsyncLogging(const std::string& basename, size_t rollSize, int flushInterval)
        : impl_(new Impl(flushInterval, false, basename, rollSize))
{
    impl_->currentPtr_->bzero();
    impl_->currentPtr_->bzero();
    impl_->buffers_.reserve(16);

}

AsyncLogging::~AsyncLogging()
{
    if (impl_->running_) {
        stop();
    }
}

void AsyncLogging::append(const char* logLine, int len)
{
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    if (impl_->currentPtr_->avail() > len) {
        impl_->currentPtr_->append(logLine, static_cast<size_t >(len));
    } else {
        impl_->buffers_.push_back(impl_->currentPtr_.release());

        if (impl_->nextBuffer_) {
            impl_->currentPtr_.reset(impl_->nextBuffer_.release());
        } else {
            impl_->currentPtr_.reset(new Buffer());
        }
        impl_->currentPtr_->append(logLine, static_cast<size_t >(len));
        impl_->cond_.notify_all();
    }

}

void AsyncLogging::start()
{
    impl_->running_ = true;
    impl_->backThread_ = std::move(std::thread(std::bind(impl_->threadFunc)));
}

void AsyncLogging::stop()
{
    impl_->running_ = false;
    impl_->cond_.notify_all();
    impl_->backThread_.join();
}

class AsyncLogging::Impl
{
public:


    Impl(int flushInterval, bool running, std::string basename, size_t rollSize)
            : flushInterval_(flushInterval), running_(running), basename_(std::move(basename)), rollSize_(rollSize),
              mutex_(), cond_(), backThread_(), currentPtr_(new Buffer()), nextBuffer_(new Buffer()), buffers_()
    {

    }

public:

    void threadFunc()
    {
        LogFile output(basename_, rollSize_);
        Buffer* newBuffer1 = new Buffer();
        Buffer* newBuffer2 = new Buffer();

        newBuffer1->bzero();
        newBuffer2->bzero();

        BufferVector buffersToWrite;
        buffersToWrite.resize(16);

        while (running_) {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (buffers_.empty()) {
                    cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
                }
                buffers_.push_back(currentPtr_.release());
                currentPtr_ = std::move(newBuffer1);
                buffersToWrite.swap(buffers_);

                if (!nextBuffer_) {
                    nextBuffer_ = std::move(newBuffer2);
                }
            }

            if (buffersToWrite.size() > 25) {
                char buff[256];
                snprintf(buff, sizeof(buff), "Dropped log message at %s, %zd larger buffers\n",
                         LogUtility::FormatTimeStampToString().c_str(), buffersToWrite.size() - 2);
                fputs(buff, stderr);
                output.append(buff, static_cast<int>(strlen(buff)));
                buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
            }


            for (auto& buff : buffersToWrite) {
                output.append(buff->data(), buff->length());
            }


            if (buffersToWrite.size() > 2) {
                buffersToWrite.resize(2);
            }

            if (!newBuffer1) {
                newBuffer1 = buffersToWrite.back().release();
                buffersToWrite.pop_back();
                newBuffer1->reset();
            }

            if (!newBuffer2) {
                newBuffer2 = buffersToWrite.back().release();
                buffersToWrite.pop_back();
                newBuffer1->reset();
            }

            buffersToWrite.clear();
            output.flush();
        }
        output.flush();
    }

public:
    const int flushInterval_;
    bool running_;
    std::string basename_;
    size_t rollSize_;

public:

    std::mutex mutex_;
    std::condition_variable cond_;
    std::thread backThread_;

public:
    BufferPtr currentPtr_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;

};
