#include "trading/strategy/MarketOrderBook.hpp"
#include "common/constants/constants.hpp"
#include "common/time/time.hpp"
#include "common/messages/MarketUpdate.hpp"

namespace trading::strategy
{
    using namespace common::constants;
    using namespace common::time;
    using namespace common::messages;

    MarketOrderBook::MarketOrderBook(TickerId tickerId, Logger* logger) : _tickerId(tickerId), _orderPool(MAX_ORDER_IDS), _ordersAtPricePool(MAX_PRICE_LEVELS), _logger(logger)
    {
        (void) _tickerId;
    }

    void MarketOrderBook::setTradeEngine(TradeEngine* tradeEngine)
    {
        _tradeEngine = tradeEngine;
    }

    void MarketOrderBook::onMarketUpdate(MarketUpdate* marketUpdate) noexcept
    {
        const auto bidUpdated = _bidsByPrice && marketUpdate->_side == Side::BUY && marketUpdate->_price >= _bidsByPrice->_price;
        const auto askUpdated = _asksByPrice && marketUpdate->_side == Side::SELL && marketUpdate->_price <= _asksByPrice->_price;

        switch (marketUpdate->_type)
        {
            using enum common::enums::MarketUpdateType;
            case ADD:
            {
                auto* order = _orderPool.allocate(marketUpdate->_orderId, marketUpdate->_side, marketUpdate->_price, marketUpdate->_qty, marketUpdate->_priority, nullptr, nullptr);
                addOrder(order);
            }
            break;
            case MODIFY:
            {
                auto* order = _oidToOrder[marketUpdate->_orderId];
                order->_qty = marketUpdate->_qty;
            }
            break;
            case CANCEL:
            {
                auto* order = _oidToOrder[marketUpdate->_orderId];
                removeOrder(order);
            }
            break;
            case TRADE:
            {
                _tradeEngine->onTrade(marketUpdate, this);
                return;
            }
            break;
            case CLEAR:
            {
                for (auto& order : _oidToOrder)
                {
                    if (order)
                    {
                        _orderPool.deallocate(order);
                    }
                }
                _oidToOrder.fill(nullptr);
                if (_bidsByPrice)
                {
                    for (auto bid = _bidsByPrice->_nextEntry; bid != _bidsByPrice; bid = bid->_nextEntry)
                    {
                        _ordersAtPricePool.deallocate(bid);
                    }
                    _ordersAtPricePool.deallocate(_bidsByPrice);
                }
                if (_asksByPrice)
                {
                    for (auto ask = _asksByPrice->_nextEntry; ask != _asksByPrice; ask = ask->_nextEntry)
                    {
                        _ordersAtPricePool.deallocate(ask);
                    }
                    _ordersAtPricePool.deallocate(_asksByPrice);
                }
                _bidsByPrice = _asksByPrice = nullptr;
            }
                break;
            case INVALID:
            case SNAPSHOT_START:
            case SNAPSHOT_END:
            break;
        }
        updateBBO(bidUpdated, askUpdated);

        _tradeEngine->onOrderBookUpdate(marketUpdate->_tickerId, marketUpdate->_price, marketUpdate->_side);
        _logger->log("%:% %() % OrderBook\n%\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), "a"/*toString(false, true)*/);
    }

    void MarketOrderBook::updateBBO(bool updateBid, bool updateAsk) noexcept
    {
        if (updateBid)
        {
            if (_bidsByPrice)
            {
                _bbo._bidPrice = _bidsByPrice->_price;
                _bbo._bidQuantity = _bidsByPrice->_firstOrder->_qty;
                for (auto order = _bidsByPrice->_firstOrder->_nextOrder; order!= _bidsByPrice->_firstOrder; order = order->_nextOrder)
                {
                    _bbo._bidQuantity += order->_qty;
                }
            }
            else
            {
                _bbo._bidPrice = PRICE_INVALID;
                _bbo._bidQuantity = QTY_INVALID;
            }
        }

        if (updateAsk)
        {
            if (_asksByPrice)
            {
                _bbo._askPrice = _asksByPrice->_price;
                _bbo._askQuantity = _asksByPrice->_firstOrder->_qty;
                for (auto order = _asksByPrice->_firstOrder->_nextOrder; order!= _asksByPrice->_firstOrder; order = order->_nextOrder)
                {
                    _bbo._askQuantity += order->_qty;
                }
            }
            else
            {
                _bbo._askPrice = PRICE_INVALID;
                _bbo._askQuantity = QTY_INVALID;
            }
        }
    }

    auto MarketOrderBook::priceToIndex(Price price) const noexcept
    {
        return price % MAX_PRICE_LEVELS;
    }

    auto MarketOrderBook::addOrdersAtPrice(MarketOrdersAtPrice* newOrdersAtPrice) noexcept
    {
        _priceOrdersAtPrice.at(priceToIndex(newOrdersAtPrice->_price)) = newOrdersAtPrice;
        const auto bestOrdersByPrice = newOrdersAtPrice->_side == Side::BUY ? _bidsByPrice : _asksByPrice;

        if (!bestOrdersByPrice) [[ unlikely ]]
        {
            (newOrdersAtPrice->_side == Side::BUY ? _bidsByPrice : _asksByPrice) = newOrdersAtPrice;
            newOrdersAtPrice->_prevEntry = newOrdersAtPrice->_nextEntry = newOrdersAtPrice;
        }
        else
        {
            auto target     = bestOrdersByPrice;
            bool addAfter   = ((newOrdersAtPrice->_side == Side::SELL   && newOrdersAtPrice->_price > target->_price) ||
                               (newOrdersAtPrice->_side == Side::BUY    && newOrdersAtPrice->_price < target->_price));
            
            if (addAfter)
            {
                target      = target->_nextEntry;
                addAfter    = ((newOrdersAtPrice->_side == Side::SELL   && newOrdersAtPrice->_price > target->_price) ||
                               (newOrdersAtPrice->_side == Side::BUY    && newOrdersAtPrice->_price < target->_price));
            }

            while (addAfter && target != bestOrdersByPrice)
            {
                addAfter    = ((newOrdersAtPrice->_side == Side::SELL   && newOrdersAtPrice->_price > target->_price) ||
                               (newOrdersAtPrice->_side == Side::BUY    && newOrdersAtPrice->_price < target->_price));
                
                if (addAfter)
                      target = target->_nextEntry;
            }
            
            if (addAfter)
            {
                if (target == bestOrdersByPrice)
                {
                    target = bestOrdersByPrice->_prevEntry;
                }
                newOrdersAtPrice->_prevEntry    = target;
                target->_nextEntry->_prevEntry  = newOrdersAtPrice;
                newOrdersAtPrice->_nextEntry    = target->_nextEntry;
                target->_nextEntry              = newOrdersAtPrice;
            }
            else
            {
                newOrdersAtPrice->_prevEntry = target->_prevEntry;
                newOrdersAtPrice->_nextEntry = target;
                newOrdersAtPrice->_prevEntry->_nextEntry = newOrdersAtPrice;
                target->_prevEntry = newOrdersAtPrice;
            }

            if ((newOrdersAtPrice->_side == Side::BUY   && newOrdersAtPrice->_price > _bidsByPrice->_price) ||
                (newOrdersAtPrice->_side == Side::SELL  && newOrdersAtPrice->_price < _asksByPrice->_price))
            {
                target->_nextEntry = (target->_nextEntry == bestOrdersByPrice ? newOrdersAtPrice : target->_nextEntry);
                (newOrdersAtPrice->_side == Side::BUY ? _bidsByPrice : _asksByPrice) = newOrdersAtPrice;
            }
        }
    }

    void MarketOrderBook::addOrder(MarketOrder* order) noexcept
    {
        const auto ordersAtPrice = getOrdersAtPrice(order->_price);

        if (!ordersAtPrice)
        {
            order->_nextOrder       = order->_prevOrder = order;
            auto newOrdersAtPrice   = _ordersAtPricePool.allocate(order->_side, order->_price, order, nullptr, nullptr);
            addOrdersAtPrice(newOrdersAtPrice);
        }
        else
        {
            auto firstOrder = ordersAtPrice->_firstOrder;
            firstOrder->_prevOrder->_nextOrder = order;
            order->_prevOrder = firstOrder->_prevOrder;
            order->_nextOrder = firstOrder;
            firstOrder->_prevOrder = order;
        }
        _oidToOrder.at(order->_orderId) = order;
    }

    auto MarketOrderBook::getOrdersAtPrice(Price price) const noexcept -> MarketOrdersAtPrice*
    {
        return _priceOrdersAtPrice.at(priceToIndex(price));
    }

    void MarketOrderBook::removeOrder(MarketOrder* order) noexcept
    {
        auto ordersAtPrice = getOrdersAtPrice(order->_price);

        if (order->_prevOrder == order)
        {
            removeOrdersAtPrice(order->_side, order->_price);
        }
        else
        {
            const auto orderBefore = order->_prevOrder;
            const auto orderAfter  = order->_nextOrder;
            orderBefore->_nextOrder = orderAfter;
            orderAfter->_prevOrder  = orderBefore;

            if (ordersAtPrice->_firstOrder == order)
            {
                ordersAtPrice->_firstOrder = orderAfter;
            }
            order->_prevOrder = order->_nextOrder = nullptr;
        }
        _oidToOrder.at(order->_orderId) = nullptr;
        _orderPool.deallocate(order);
    }

    void MarketOrderBook::removeOrdersAtPrice(Side side, Price price) noexcept
    {
        const auto bestOrdersAtPrice    = (side == Side::BUY ? _bidsByPrice : _asksByPrice);
        auto ordersAtPrice              = getOrdersAtPrice(price);

        if (ordersAtPrice->_nextEntry == ordersAtPrice) [[ unlikely ]]
        {
            (side == Side::BUY ? _bidsByPrice : _asksByPrice) = nullptr;
        }
        else
        {
            ordersAtPrice->_prevEntry->_nextEntry = ordersAtPrice->_nextEntry;
            ordersAtPrice->_nextEntry->_prevEntry = ordersAtPrice->_prevEntry;
            
            if (ordersAtPrice == bestOrdersAtPrice)
            {
                (ordersAtPrice->_side == Side::BUY ? _bidsByPrice : _asksByPrice) = ordersAtPrice->_nextEntry;
            }
            ordersAtPrice->_nextEntry = ordersAtPrice->_prevEntry = nullptr;
        }

        _priceOrdersAtPrice.at(priceToIndex(price)) = nullptr;
        _ordersAtPricePool.deallocate(ordersAtPrice);

    }

    MarketOrderBook::~MarketOrderBook()
    {
        _logger->log("%:% %() % OrderBook\n%\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), "a"/*toString(false, true)*/);
        
        _tradeEngine = nullptr;
        _bidsByPrice = _asksByPrice = nullptr;
        _oidToOrder.fill(nullptr);
    }
}