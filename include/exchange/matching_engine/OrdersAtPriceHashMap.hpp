#pragma once

#include <array>
#include "OrdersAtPrice.hpp"
#include "constants/constants.hpp"

namespace exchange::matching_engine
{
    using OrdersAtPriceHashMap = std::array<OrdersAtPrice *, constants::MAX_PRICE_LEVELS>;
} // namespace exchange::matching_engine
