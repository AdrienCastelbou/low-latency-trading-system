#pragma once
#include <cstdint>
#include <string>

namespace common::enums
{
    enum class MarketUpdateType
    {
        INVALID = 0,
        ADD     = 1,
        MODIFY  = 2,
        CANCEL  = 3,
        TRADE   = 4
    };

    inline std::string marketUpdateTypeToString(MarketUpdateType type)
    {
        switch (type)
        {
            case MarketUpdateType::ADD:
                return "ADD";
            case MarketUpdateType::MODIFY:
                return "MODIFY";
            case MarketUpdateType::CANCEL:
                return "CANCEL";
            case MarketUpdateType::TRADE:
                return "TRADE";
            case MarketUpdateType::INVALID:
                return "INVALID";
        }
        return "UNKNOWN";
  }
} // namespace exchange::common::enums
