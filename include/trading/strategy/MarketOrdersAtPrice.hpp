#pragma once
#include "../../common/types/types.hpp"
#include "MarketOrder.hpp"

namespace trading::strategy
{
    using namespace common::types;

    struct MarketOrder;

    struct MarketOrdersAtPrice
    {
        MarketOrdersAtPrice() = default;
        MarketOrdersAtPrice(Side side, Price price, MarketOrder* firstOrder, MarketOrdersAtPrice* prevEntry, MarketOrdersAtPrice* nextEntry);
        
        std::string toString() const;

        Side _side = Side::INVALID;
        Price _price = PRICE_INVALID;
        MarketOrder* _firstOrder = nullptr;
        MarketOrdersAtPrice* _nextEntry = nullptr;
        MarketOrdersAtPrice* _prevEntry = nullptr;
    };
}