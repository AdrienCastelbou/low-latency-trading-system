#pragma once
#include "Order.hpp"
#include "../../../common/types/types.hpp"

namespace trading::strategy::order_management
{
    using namespace common::types;
    
    using OrderSideHashMap = std::array<Order, sideToIndex(Side::MAX) + 1>;
}