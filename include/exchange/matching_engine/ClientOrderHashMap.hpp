#pragma once
#include <array>
#include "OrderHashMap.hpp"

namespace exchange::matching_engine
{
    using ClientOrderHashMap = std::array<OrderHashMap, constants::MAX_NUM_CLIENTS>;
} // namespace exchange::matching_engine
