#pragma once
#include <uchar.h>

namespace exchange::matching_engine::constants
{
    constexpr size_t MAX_TICKERS        = 8;
    constexpr size_t MAX_CLIENT_UPDATES = 256 * 1024;
    constexpr size_t MAX_MARKET_UPDATES = 256 * 1024;
    constexpr size_t MAX_NUM_CLIENTS    = 256;
    constexpr size_t MAX_ORDER_IDS      = 1024 * 1024;
    constexpr size_t MAX_PRICE_LEVELS   = 256;
  
} // namespace exchange::matching_engine::constants
