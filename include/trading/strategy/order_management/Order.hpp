#pragma once
#include "../../../common/types/types.hpp"
#include "OrderState.hpp"

namespace trading::strategy::order_management
{
    using namespace common::types;

    struct Order
    {
        TickerId _tickerId = TICKERID_INVALID;
        OrderId _orderId = ORDERID_INVALID;
        Side _side = Side::INVALID;
        Price _price = PRICE_INVALID;
        Qty _qty = QTY_INVALID;
        OrderState _orderState = OrderState::INVALID;

        std::string toString() const;
    };
}