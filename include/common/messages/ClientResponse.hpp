#pragma once

#include "../enums/ClientResponseType.hpp"
#include "../types/OrderTypes.hpp"

using namespace common::types;

namespace common::messages
{
    #pragma pack(push, 1)
    struct ClientResponse
    {
        enums::ClientResponseType _type = enums::ClientResponseType::INVALID;
        ClientId _clientId              = CLIENTID_INVALID;
        TickerId _tickerId              = TICKERID_INVALID;
        OrderId _clientOrderId          = ORDERID_INVALID;
        OrderId _marketOrderId          = ORDERID_INVALID;
        Side    _side                   = Side::INVALID;
        Price   _price                  = PRICE_INVALID;
        Qty     _execQty                = QTY_INVALID;
        Qty     _leavesQty              = QTY_INVALID;
        std::string toString() const;
    };
    #pragma pack(pop)    
} // namespace common::network::messages
