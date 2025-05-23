#pragma once

#include <chrono>
#include <ctime>
#include <string>
#include "../performance/performanceUtils.hpp"

namespace common::time
{
    using Nanos = int64_t;
    
    constexpr Nanos NANOS_TO_MICROS     = 1000;
    constexpr Nanos MICROS_TO_MILLIS    = 1000;
    constexpr Nanos MILLIS_TO_SECS      = 1000;
    constexpr Nanos NANOS_TO_MILLIS     = NANOS_TO_MICROS * MICROS_TO_MILLIS;
    constexpr Nanos NANOS_TO_SECS       = NANOS_TO_MILLIS * MILLIS_TO_SECS;

    inline auto getCurrentNanos() noexcept
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    inline auto& getCurrentTimeStr(std::string* timeStr)
    {
        const auto clock = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(clock);
        char nanosStr[24] = {0};

        snprintf(nanosStr, sizeof(nanosStr), "%.8s.%09lld", ctime(&time) + 11, std::chrono::duration_cast<std::chrono::nanoseconds>(clock.time_since_epoch()).count() % NANOS_TO_SECS);
        timeStr->assign(nanosStr);

        return *timeStr;
    }
} // namespace common::time
