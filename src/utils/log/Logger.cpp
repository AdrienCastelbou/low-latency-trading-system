#include "utils/logging/Logger.hpp"
#include "utils/utils.hpp"
#include "threading/thread_utils.hpp"

namespace utils::logging
{
    Logger::Logger(const std::string& filename) : _filename(filename), _queue(LOG_QUEUE_SIZE), _running(true), _thread(nullptr)
    {
        _file.open(_filename);
        utils::assert(_file.is_open(), "Could not open log file : " + _filename);
        _thread = threading::createAndStartThread(-1, "Logger", [this]() { flushQueue();});
        utils::assert(_thread != nullptr, "Failed to start logger thread");
    }

    Logger::~Logger()
    {
        std::cerr << "Flushing and closing Logger for " << _filename << std::endl;

        while (_queue.size())
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
        }
        _running = false;
        _thread->join();
        _file.close();
    }

    void Logger::flushQueue()
    {
        while (_running)
        {
            for (auto next = _queue.getNextToRead(); _queue.size() && next; next = _queue.getNextToRead())
            {
                switch (next->_type)
                {
                case LogType::CHAR:
                    _file << next->_u.c;
                    break;
                case LogType::INTEGER:
                    _file << next->_u.i;
                    break;
                case LogType::LONG_INTEGER:
                    _file << next->_u.l;
                    break;
                case LogType::LONG_LONG_INTEGER:
                    _file << next->_u.ll;
                    break;
                case LogType::UNSIGNED_INTEGER:
                    _file << next->_u.u;
                    break;
                case LogType::UNSIGNED_LONG_INTEGER:
                    _file << next->_u.ul;
                    break;
                    case LogType::UNSIGNED_LONG_LONG_INTEGER:
                    _file << next->_u.ull;
                    break;
                case LogType::FLOAT:
                    _file << next->_u.f;
                    break;
                case LogType::DOUBLE:
                    _file << next->_u.d;
                    break;
                }

                _queue.updateReadIndex();
            }
            using namespace std::literals::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }
    }

    void Logger::pushValue(const LogElement& logElem) noexcept
    {
        *(_queue.getNextToWriteTo()) = logElem;
        _queue.updateWriteIndex();
    }

    void Logger::pushValue(const std::string& value) noexcept
    {
        pushValue(value.c_str());
    }

    void Logger::pushValue(const char* value) noexcept
    {
        while (*value)
        {
            pushValue(*value);
            value++;
        }
    }

    void Logger::pushValue(const char value) noexcept
    {
        pushValue(LogElement{LogType::CHAR, {.c = value}});
    }
    
    void Logger::pushValue(const int value) noexcept
    {
        pushValue(LogElement{LogType::INTEGER, {.i = value}});
    }

    void Logger::pushValue(const long value) noexcept
    {
        pushValue(LogElement{LogType::LONG_INTEGER, {.l = value}});
    }

    void Logger::pushValue(const long long value) noexcept
    {
        pushValue(LogElement{LogType::LONG_LONG_INTEGER, {.ll = value}});
    }

    void Logger::pushValue(const unsigned value) noexcept
    {
        pushValue(LogElement{LogType::UNSIGNED_INTEGER, {.u = value}});
    }

    void Logger::pushValue(const unsigned long value) noexcept
    {
        pushValue(LogElement{LogType::UNSIGNED_LONG_INTEGER, {.ul = value}});
    }

    void Logger::pushValue(const unsigned long long value) noexcept
    {
        pushValue(LogElement{LogType::UNSIGNED_LONG_LONG_INTEGER, {.ull = value}});
    }

    void Logger::pushValue(const float value) noexcept
    {
        pushValue(LogElement{LogType::FLOAT, {.f = value}});
    }

    void Logger::pushValue(const double value) noexcept
    {
        pushValue(LogElement{LogType::DOUBLE, {.d = value}});
    }

    void Logger::log(const char* s) noexcept
    {
        while (*s)
        {
            if (*s == '%')
            {
                if (*(s + 1) == '%') [[ unlikely ]]
                {
                    ++s;
                }
                else
                {
                    utils::fatal("Missing arguments to Logger::log()");
                }
            }
            pushValue(*s++);
        }
    }
} // namespace utils::log
