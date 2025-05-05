#pragma once

#include <array>
#include "constants/constants.hpp"
#
namespace exchange::matching_engine
{
    struct Order;

    using OrderHashMap = std::array<Order*, constants::MAX_ORDER_IDS>;
} // namespace exchange::matching_engine
