#include "trading/strategy/MarketOrder.hpp"
#include <sstream>
namespace trading::strategy
{
    MarketOrder::MarketOrder(OrderId orderId, Side side, Price price, Qty qty, Priority priority, MarketOrder* prevOrder, MarketOrder* nextOrder) noexcept
    : _orderId(orderId), _side(side), _price(price), _qty(qty), _priority(priority), _prevOrder(prevOrder), _nextOrder(nextOrder)
    {
    }

    std::string MarketOrder::toString() const
    {
        std::stringstream ss;
        ss << "MarketOrder" << "["
           << "oid:" << orderIdToString(_orderId) << " "
           << "side:" << sideToString(_side) << " "
           << "price:" << priceToString(_price) << " "
           << "qty:" << qtyToString(_qty) << " "
           << "prio:" << priorityToString(_priority) << " "
           << "prev:" << orderIdToString(_prevOrder ? _prevOrder->_orderId : ORDERID_INVALID) << " "
           << "next:" << orderIdToString(_nextOrder ? _nextOrder->_orderId : ORDERID_INVALID) << "]";
        return ss.str();
    }
}