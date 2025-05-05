#pragma once
#include <cstdint>
#include <string>

namespace common::enums
{
    enum class MarketUpdateType
    {
        INVALID = 0,
        CLEAR = 1,
        ADD = 2,
        MODIFY = 3,
        CANCEL = 4,
        TRADE = 5,
        SNAPSHOT_START = 6,
        SNAPSHOT_END = 7
    };

    inline std::string marketUpdateTypeToString(MarketUpdateType type)
    {
        switch (type)
        {
            case MarketUpdateType::CLEAR:
                return "CLEAR";
            case MarketUpdateType::ADD:
                return "ADD";
            case MarketUpdateType::MODIFY:
                return "MODIFY";
            case MarketUpdateType::CANCEL:
                return "CANCEL";
            case MarketUpdateType::TRADE:
                return "TRADE";
            case MarketUpdateType::SNAPSHOT_START:
                return "SNAPSHOT_START";
            case MarketUpdateType::SNAPSHOT_END:
                return "SNAPSHOT_END";
            case MarketUpdateType::INVALID:
                return "INVALID";
        }
        return "UNKNOWN";
  }
} // namespace exchange::common::enums
