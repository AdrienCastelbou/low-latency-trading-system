#pragma once

namespace common::performance
{
    inline auto rdtsc() noexcept
    {
        unsigned int lo, hi;
        
        asm volatile ("rdtsc" : "=r" (lo), "=r" (hi));
        return ((uint64_t)hi << 32) | lo;
    }

    #define START_MEASURE(TAG) const auto TAG = rdtsc();

    #define END_MEASURE(TAG, LOGGER) \
        do { \
            const auto end = rdtsc(); \
            LOGGER.log("% RDTSC " #TAG "%\n", common::time::getCurrentTimeStr(&_timeStr), (end - TAG));\
        } while (false);
        
    #define TTT_MEASURE(TAG, LOGGER) \
        do { \
            const auto TAG = common::time::getCurrentNanos(); \
            LOGGER.log("% TTT " #TAG "%\n", common::time::getCurrentTimeStr(&_timeStr), TAG);\
        } while (false);
}