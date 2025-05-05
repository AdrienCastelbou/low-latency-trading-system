#include "common/messages/ClientResponse.hpp"
#include <sstream>

using namespace ::common::types;

namespace common::messages
{
    std::string ClientResponse::toString() const
    {
        std::stringstream ss;
        
        ss << "lientRequest"
        << " ["
        << "type:" << enums::clientResponseTypeToString(_type)
        << " client:" << clientIdToString(_clientId)
        << " ticker:" << tickerIdToString(_tickerId)
        << " coid:" << orderIdToString(_clientOrderId)
        << " moid:" << orderIdToString(_marketOrderId)
        << " side:" << sideToString(_side)
        << " exec_qty:" << qtyToString(_execQty)
        << " leaves_qty:" << qtyToString(_leavesQty)
        << " price:" << priceToString(_price)
        << "]";
     return ss.str();
    }
} // namespace common::messages
