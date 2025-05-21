#include "common/messages/ClientRequest.hpp"
#include <sstream>

namespace common::messages
{
    std::string ClientRequest::toString() const
    {
        using namespace ::common::types;
        std::stringstream ss;
        
        ss << "clientRequest"
        << " ["
        << "type:" << enums::clientRequestTypeToString(_type)
        << " client:" << clientIdToString(_clientId)
        << " ticker:" << tickerIdToString(_tickerId)
        << " oid:" << orderIdToString(_orderId)
        << " side:" << sideToString(_side)
        << " qty:" << qtyToString(_qty)
        << " price:" << priceToString(_price)
        << "]";
     return ss.str();
    }
} // namespace common::messages
