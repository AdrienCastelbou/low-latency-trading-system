#pragma once

#include <array>
#include "../../common/constants/constants.hpp"

namespace exchange::matching_engine
{
    struct Order;

    using OrderHashMap = std::array<Order*, common::constants::MAX_ORDER_IDS>;
} // namespace exchange::matching_engine
