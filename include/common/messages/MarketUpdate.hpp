#pragma once

#include "../enums/MarketUpdateType.hpp"
#include "../types/OrderTypes.hpp"

using namespace common::types;

namespace common::messages
{
    #pragma pack(push, 1)
    struct MarketUpdate {
        enums::MarketUpdateType _type   = enums::MarketUpdateType::INVALID;
        OrderId _orderId                = ORDERID_INVALID;
        TickerId _tickerId              = TICKERID_INVALID;
        Side _side                      = Side::INVALID;
        Price _price                    = PRICE_INVALID;
        Qty _qty                        = QTY_INVALID;
        Priority _priority              = PRIORITY_INVALID;

        std::string toString() const;
      };
    #pragma pack(pop)
} // namespace common::network::messages
