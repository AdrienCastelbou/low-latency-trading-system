#pragma once
#include "../../common/types/OrderTypes.hpp"
#include <string>

namespace trading::strategy
{
    using namespace common::types;

    struct MarketOrder
    {
        MarketOrder() = default;
        MarketOrder(OrderId orderId, Side side, Price price, Qty qty, Priority priority, MarketOrder* prevOrder, MarketOrder* nextOrder) noexcept;
        
        std::string toString() const;
        
        OrderId _orderId = ORDERID_INVALID;
        Side _side = Side::INVALID;
        Price _price = PRICE_INVALID;
        Qty _qty = QTY_INVALID;
        Priority _priority = PRIORITY_INVALID;
        MarketOrder* _prevOrder = nullptr;
        MarketOrder* _nextOrder = nullptr;
    };
}