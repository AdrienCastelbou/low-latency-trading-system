#pragma once
#include "../../common/types/OrderTypes.hpp"

using namespace common::types;

namespace exchange::matching_engine
{
    struct Order
    {
        TickerId _tickerId      = TICKERID_INVALID;
        ClientId _clientId      = CLIENTID_INVALID;
        OrderId _clientOrderId  = ORDERID_INVALID;
        OrderId _marketOrderId  = ORDERID_INVALID;
        Side _side              = Side::INVALID;
        Price _price            = PRICE_INVALID;
        Qty _qty                = QTY_INVALID;
        Priority _priority      = PRIORITY_INVALID;
        Order* _prevOrder       = nullptr;
        Order* _nextOrder       = nullptr;

        Order() = default;
        Order(TickerId tickerId, ClientId ClientId, OrderId clientOrderId, OrderId marketOrderId, Side side, Price price, 
              Qty qty, Priority priority, Order* prevOrder, Order* nextOrder);

        std::string toString() const;
    };
    
} // namespace exchange::matching_engine
