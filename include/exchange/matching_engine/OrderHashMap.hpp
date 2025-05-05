#pragma once

#include <array>
#include "../shared/constants/constants.hpp"

namespace exchange::matching_engine
{
    struct Order;

    using OrderHashMap = std::array<Order*, exchange::shared::constants::MAX_ORDER_IDS>;
} // namespace exchange::matching_engine
