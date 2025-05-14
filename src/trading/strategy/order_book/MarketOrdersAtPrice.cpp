#include "trading/strategy/order_book/MarketOrdersAtPrice.hpp"
#include <sstream>

namespace trading::strategy::order_book
{
    MarketOrdersAtPrice::MarketOrdersAtPrice(Side side, Price price, MarketOrder* firstOrder, MarketOrdersAtPrice* prevEntry, MarketOrdersAtPrice* nextEntry)
        : _side(side), _price(price), _firstOrder(firstOrder), _nextEntry(nextEntry), _prevEntry(prevEntry)
    {
    }

    std::string MarketOrdersAtPrice::toString() const
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
}