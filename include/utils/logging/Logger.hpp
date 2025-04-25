#pragma once
#include <string>
#include <fstream>
#include "../LFQueue.hpp"
#include "LogElement.hpp"
#include <atomic>
#include <thread>

namespace utils::logging
{
    constexpr size_t LOG_QUEUE_SIZE = 8 * 1024 * 1024;

    class Logger final
    {
        public:
            Logger() = delete;
            Logger(const std::string& filename);
            ~Logger();
            Logger(const Logger &) = delete;
            Logger(const Logger &&) = delete;
            Logger &operator=(const Logger &) = delete;
            Logger &operator=(const Logger &&) = delete;

            template<typename T, typename... A>
            void log(const char* s, const T& value, A... args) noexcept;
            
            void log(const char* s) noexcept;

            void pushValue(const LogElement& logElem) noexcept;
            void pushValue(const std::string& value) noexcept;
            void pushValue(const char* value) noexcept;
            void pushValue(const char value) noexcept;
            void pushValue(const int value) noexcept;
            void pushValue(const long value) noexcept;
            void pushValue(const long long value) noexcept;
            void pushValue(const unsigned value) noexcept;
            void pushValue(const unsigned long value) noexcept;
            void pushValue(const unsigned long long value) noexcept;
            void pushValue(const float value) noexcept;
            void pushValue(const double value) noexcept;

        private:
            void flushQueue();
            
            const std::string   _filename;
            std::ofstream       _file;
            utils::LFQueue<LogElement> _queue;
            std::atomic_bool            _running;
            std::thread* _thread;
    };

    template<typename T, typename... A>
    void Logger::log(const char* s, const T& value, A... args) noexcept
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
                    pushValue(value);
                    log(s + 1, args...);
                    return;
                }
            }
            pushValue(*s++);
        }
        utils::fatal("Extra arguments provided to Logger::log()");
    }
} // namespace utils::log
