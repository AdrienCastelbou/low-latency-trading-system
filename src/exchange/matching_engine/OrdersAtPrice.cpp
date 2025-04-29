#include "exchange/matching_engine/OrdersAtPrice.hpp"
#include <sstream>

using namespace common::types;

namespace exchange::matching_engine
{
    OrdersAtPrice::OrdersAtPrice(Side side, Price price, Order* firstOrder, OrdersAtPrice* prevEntry, OrdersAtPrice* nextEntry)
                                : _side(side), _price(price), _firstOrder(firstOrder), _prevEntry(prevEntry), _nextEntry(nextEntry)
    {
    }
    
    std::string OrdersAtPrice::toString() const
    {
        std::stringstream ss;
        ss << "MEOrdersAtPrice["
           << "side:" << sideToString(_side) << " "
           << "price:" << priceToString(_price) << " "
           << "first_me_order:" << (_firstOrder ?
             _firstOrder->toString() : "null") << " "
           << "prev:" << priceToString(_prevEntry ?
             _prevEntry->_price : PRICE_INVALID) << " "
           << "next:" << priceToString(_nextEntry ?
             _nextEntry->_price : PRICE_INVALID) << "]";
        return ss.str();
    }
} // namespace exchange::matching_engine@
