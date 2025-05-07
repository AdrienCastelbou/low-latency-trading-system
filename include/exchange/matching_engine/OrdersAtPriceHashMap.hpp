#pragma once

#include <array>
#include "OrdersAtPrice.hpp"
#include "../../common/constants/constants.hpp"

namespace exchange::matching_engine
{
    using OrdersAtPriceHashMap = std::array<OrdersAtPrice *, common::constants::MAX_PRICE_LEVELS>;
} // namespace exchange::matching_engine
