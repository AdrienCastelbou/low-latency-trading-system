#pragma once

#include "MarketUpdate.hpp"

namespace common::messages
{
    #pragma pack(push, 1)
    struct MDPMarketUpdate {
        size_t _seqNum = 0;
        MarketUpdate _marketUpdate;
        std::string toString() const;
      };
    #pragma pack(pop)
} // namespace common::messages