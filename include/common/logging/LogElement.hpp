#pragma once
#include "LogType.hpp"

namespace common::logging
{
    struct LogElement
    {
        LogType _type = LogType::CHAR;
        union 
        {
            char c;
            int i;
            long l;
            long long ll;
            unsigned u;
            unsigned long ul;
            unsigned long long ull;
            float f;
            double d;
        } _u;
    };
    
} // namespace common::log
