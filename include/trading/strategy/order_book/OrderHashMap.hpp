#pragma once

#include "../../../common/constants/constants.hpp"

namespace trading::strategy::order_book
{
    struct MarketOrder;
    using OrderHashMap = std::array<MarketOrder*, common::constants::MAX_ORDER_IDS>;
}