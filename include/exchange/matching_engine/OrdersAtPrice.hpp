#pragma once
#include "Order.hpp"
#include "../../common/types/types.hpp"

using namespace common::types;

namespace exchange::matching_engine
{
    struct OrdersAtPrice
    {
        Side _side = Side::INVALID;
        Price _price = PRICE_INVALID;
        Order* _firstOrder = nullptr;
        OrdersAtPrice* _prevEntry = nullptr;
        OrdersAtPrice* _nextEntry = nullptr;

        OrdersAtPrice() = default;
        OrdersAtPrice(Side side, Price price, Order* firstOrder, OrdersAtPrice* prevEntry, OrdersAtPrice* nextEntry);

        std::string toString() const;
    };
    
} // namespace exchange::matching_engine
