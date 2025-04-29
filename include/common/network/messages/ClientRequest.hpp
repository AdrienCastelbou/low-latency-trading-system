#pragma once

#include "../../types/OrderTypes.hpp"
#include "../../enums/ClientRequestType.hpp"

using namespace common::types;

namespace common::network::messages
{
    #pragma pack(push, 1)
    struct ClientRequest
    {
        enums::ClientRequestType _type  = enums::ClientRequestType::INVALID;
        ClientId _clientId              = CLIENTID_INVALID;
        TickerId _tickerId              = TICKERID_INVALID;
        OrderId _orderId                = ORDERID_INVALID;
        Side    _side                   = Side::INVALID;
        Price   _price                  = PRICE_INVALID;
        Qty     _qty                    = QTY_INVALID;
        std::string toString() const;
    };
    #pragma pack(pop)

} // namespace common::network::messages
