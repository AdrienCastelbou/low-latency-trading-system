#pragma once

#include "common/types/types.hpp"
#include <sstream>

namespace trading::strategy
{
    using namespace common::types;

    struct BBO
    {
        Price _bidPrice = PRICE_INVALID;
        Price _askPrice = PRICE_INVALID;
        Qty _bidQuantity = QTY_INVALID;
        Qty _askQuantity = QTY_INVALID;

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
    };
}