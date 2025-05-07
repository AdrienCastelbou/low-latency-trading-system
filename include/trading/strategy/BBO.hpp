#pragma once

#include "common/types/OrderTypes.hpp"

namespace trading::strategy
{
    using common::types;

    struct BBO
    {
        Price _bidPrice = PRICE_INVALID;
        Price _askPrice = PRICE_INVALID;
        Quantity _bidQuantity = QUANTITY_INVALID;
        Quantity _askQuantity = QUANTITY_INVALID;

        std::string toString() const
        {
            std::stringstream ss;
            ss << "BBO{"
                << qtyToString(_bidQuantity)
                << "@" << priceToString(_bidPrice)
                << "X"
                << priceToString(_askPrice)
                << "@" << qtyToString(_askQuantity)
                << "}";
            return ss.str();
        }
    }
}