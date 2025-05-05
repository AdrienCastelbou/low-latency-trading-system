#include "common/messages/MarketUpdate.hpp"
#include <sstream>

using namespace ::common::types;

namespace common::messages
{
    std::string MarketUpdate::toString() const
    {
        std::stringstream ss;
        ss << "MarketUpdate"
           << " ["
           << " type:" << enums::marketUpdateTypeToString(_type)
           << " ticker:" << tickerIdToString(_tickerId)
           << " oid:" << orderIdToString(_orderId)
           << " side:" << sideToString(_side)
           << " qty:" << qtyToString(_qty)
           << " price:" << priceToString(_price)
           << " priority:" << priorityToString(_priority)
           << "]";
        return ss.str();
    }
} // namespace common::messages
