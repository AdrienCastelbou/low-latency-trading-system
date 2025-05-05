#pragma once
#include "OrderBook.hpp"
#include <array>

namespace exchange::matching_engine
{
    using OrderBookHashMap = std::array<OrderBook*, exchange::shared::constants::MAX_TICKERS>;
} // namespace exchange::matching_engine
