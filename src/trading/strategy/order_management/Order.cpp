#include "trading/strategy/order_management/Order.hpp"
#include <sstream>

namespace trading::strategy::order_management
{
    std::string Order::toString() const
    {
        std::stringstream ss;
        
        ss << "OM Order : " << "["
            << "tid" << tickerIdToString(_tickerId) << " "
            << "oid" << orderIdToString(_orderId) << " "
            << "side" << sideToString(_side) << " "
            << "price" << priceToString(_price) << " "
            << "qty" << qtyToString(_qty) << " "
            << "state" << OrderStateToString(_state)
            << "]";
        return ss.str();
    }
}