#include "log_content_buffer_sink.h"

#include <mutex>

namespace logging_test {
LogContentBufferSink::LogContentBufferSink() : LogContentBufferSink(1024)
{
    _paramStr = std::string("LogContentBufferSink, Capacity: ") + std::to_string(_capacity);
}

LogContentBufferSink::LogContentBufferSink(uint32_t capacity) : _capacity(capacity)
{
    _buffer.reserve(_capacity);
}

uint32_t LogContentBufferSink::capacity() const
{
    return _capacity;
}

const std::vector<std::string>& LogContentBufferSink::buffer()
{
    std::lock_guard<std::mutex> lock(_sinkMtx);
    return _buffer;
}

const std::vector<std::string>& LogContentBufferSink::disk()
{
    std::lock_guard<std::mutex> lock(_sinkMtx);
    return _disk;
}

void LogContentBufferSink::clear()
{
    _buffer.clear();
    _disk.clear();
}

void LogContentBufferSink::log_it(const LogMsg& logMsg)
{
    std::string content;
    _formatter->format(logMsg, content);
    sink_it(content);
}

void LogContentBufferSink::sink_it(std::string_view message)
{
    _buffer.emplace_back(message);
    if (_buffer.size() >= _capacity) {
        flush_it();
    }
}

void LogContentBufferSink::flush_it()
{
    _disk.reserve(_disk.size() + _buffer.size());
    _disk.insert(_disk.end(), _buffer.begin(), _buffer.end());
    _buffer.clear();
}

}  // namespace logging_test
