#pragma once
#include "OrderSideHashMap.hpp"
#include "../../../common/constants/constants.hpp"

namespace trading::strategy::order_management
{
    using namespace common::constants;

    using OrderTickerSideHashMap = std::array<OrderSideHashMap, MAX_TICKERS>;
}