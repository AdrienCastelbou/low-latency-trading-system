#pragma once

namespace common::performance
{
    inline auto rdtsc() noexcept
    {
        uint64_t val;
        asm volatile("mrs %0, cntvct_el0" : "=r"(val));
        return val;
    }

    #define START_MEASURE(TAG) const auto TAG = common::performance::rdtsc();

    #define END_MEASURE(TAG, LOGGER) \
        do { \
            const auto end = common::performance::rdtsc(); \
            LOGGER.log("% RDTSC " #TAG "%\n", common::time::getCurrentTimeStr(&_timeStr), (end - TAG));\
        } while (false);
        
    #define TTT_MEASURE(TAG, LOGGER) \
        do { \
            const auto TAG = common::time::getCurrentNanos(); \
            LOGGER.log("% TTT " #TAG "%\n", common::time::getCurrentTimeStr(&_timeStr), TAG);\
        } while (false);
}