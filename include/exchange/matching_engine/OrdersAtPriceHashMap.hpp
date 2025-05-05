#pragma once

#include <array>
#include "OrdersAtPrice.hpp"
#include "../shared/constants/constants.hpp"

namespace exchange::matching_engine
{
    using OrdersAtPriceHashMap = std::array<OrdersAtPrice *, exchange::shared::constants::MAX_PRICE_LEVELS>;
} // namespace exchange::matching_engine
