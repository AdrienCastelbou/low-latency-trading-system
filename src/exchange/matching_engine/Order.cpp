#include "exchange/matching_engine/Order.hpp"
#include <sstream>

using namespace common::types;

namespace exchange::matching_engine
{
    Order::Order(TickerId tickerId, ClientId ClientId, OrderId clientOrderId, OrderId marketOrderId, Side side, Price price, 
                Qty qty, Priority priority, Order* prevOrder, Order* nextOrder)
                :   _tickerId(tickerId), _clientId(ClientId), _clientOrderId(clientOrderId), _marketOrderId(marketOrderId),
                    _side(side), _price(price), _qty(qty), _priority(priority), _prevOrder(prevOrder), _nextOrder(nextOrder)
    {
    }

    std::string Order::toString() const
    {
        std::stringstream ss;
        
        ss << "Order"
        << " ["
        << " client:" << clientIdToString(_clientId)
        << " ticker:" << tickerIdToString(_tickerId)
        << " coid:" << orderIdToString(_clientOrderId)
        << " moid:" << orderIdToString(_marketOrderId)
        << " side:" << sideToString(_side)
        << " price:" << priceToString(_price)
        << " qty:" << qtyToString(_qty)
        << " prio:" << qtyToString(_priority)
        << " prev:" << orderIdToString(_prevOrder ? _prevOrder->_marketOrderId : ORDERID_INVALID)
        << " next:" << orderIdToString(_nextOrder ? _nextOrder->_marketOrderId : ORDERID_INVALID)
        << "]";
     return ss.str();
    }
} // namespace exchange::matching_engine
