#pragma once

#include "common/constants/constants.hpp"

namespace trading::strategy
{
    struct MarketOrdersAtPrice;
    using OrdersAtPriceHashMap = std::array<MarketOrdersAtPrice*, common::constants::MAX_PRICE_LEVELS>;
} // namespace trading::strategy