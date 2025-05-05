#include "exchange/matching_engine/OrderBook.hpp"
#include "common/logging/Logger.hpp"
#include "common/time/time.hpp"
#include "exchange/matching_engine/MatchingEngine.hpp"
#include "exchange/matching_engine/constants/constants.hpp"
#include "common/enums/ClientResponseType.hpp"

namespace cnsts = exchange::matching_engine::constants;

namespace exchange::matching_engine
{
    OrderBook::OrderBook(TickerId tickerId, ::common::logging::Logger* logger, MatchingEngine* matchingEngine)
    : _tickerId(tickerId), _matchingEngine(matchingEngine), _ordersAtPricePool(cnsts::MAX_PRICE_LEVELS),
    _orderPool(cnsts::MAX_ORDER_IDS), _logger(logger) 
    {
    }

    OrderBook::~OrderBook()
    {
        _logger->log("%:% %() % OrderBook\n%\n",
                    __FILE__, __LINE__, __FUNCTION__, ::common::time::getCurrentTimeStr(&_timeStr), true/*toString(false, true)*/);
        
        _matchingEngine = nullptr;
        _bidsByPrice    = nullptr;
        _asksByPrice    = nullptr;
        for (auto& itr : _cidOidToOrder)
        {
            itr.fill(nullptr);
        }
    }

    auto OrderBook::priceToIndex(Price price) const noexcept
    {
        return price % cnsts::MAX_PRICE_LEVELS;
    }

    auto OrderBook::getNextPriority(Price price) noexcept
    {
        const auto ordersAtPrice = getOrdersAtPrice(price);
        
        if (!ordersAtPrice)
          return 1llu;
        
          return ordersAtPrice->_firstOrder->_prevOrder->_priority + 1;
      }

    auto OrderBook::addOrdersAtPrice(OrdersAtPrice* newOrdersAtPrice) noexcept
    {
        _ordersAtPrice.at(priceToIndex(newOrdersAtPrice->_price)) = newOrdersAtPrice;
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

    auto OrderBook::addOrder(Order* order) noexcept
    {
        const auto ordersAtPrice = getOrdersAtPrice(order->_price);

        if (!ordersAtPrice)
        {
            order->_nextOrder = order->_prevOrder = order;
            auto newOrdersAtPrice = _ordersAtPricePool.allocate(order->_side, order->_priority, order, nullptr, nullptr);
            addOrdersAtPrice(newOrdersAtPrice);
        }
        else
        {
            auto firstOrder                     = ordersAtPrice ? ordersAtPrice->_firstOrder : nullptr;
            firstOrder->_prevOrder->_nextOrder  = order;
            order->_prevOrder                   = firstOrder->_prevOrder;
            order->_nextOrder                   = firstOrder;
            firstOrder->_prevOrder              = order;
        }
        _cidOidToOrder.at(order->_clientId).at(order->_clientOrderId) = order;
    }

    auto OrderBook::match(TickerId tickerId, ClientId clientId, Side side, OrderId clientOrderId, OrderId newMarketOrderId, Order* itr, Qty* leftQty) noexcept
    {
        using namespace ::common::enums;

        const auto order    = itr;
        const auto orderQty = order->_qty;
        const auto fillQty  = std::min(*leftQty, orderQty);

        *leftQty -= fillQty;
        order->_qty -= fillQty;
        
        _clientResponse = { ClientResponseType::FILLED, clientId, tickerId, clientOrderId, newMarketOrderId, side, itr->_price, fillQty, *leftQty};
        _matchingEngine->sendClientResponse(&_clientResponse);
        _clientResponse = { ClientResponseType::FILLED, order->_clientId, tickerId, order->_clientOrderId, order->_marketOrderId, order->_side, order->_price, fillQty, order->_qty};
        _matchingEngine->sendClientResponse(&_clientResponse);
        _marketUpdate = { MarketUpdateType::TRADE, ORDERID_INVALID, tickerId, side, itr->_price, fillQty, PRIORITY_INVALID};
        _matchingEngine->sendMarketUpdate(&_marketUpdate);
        
        if (!order->_qty)
        {
            _marketUpdate = { MarketUpdateType::CANCEL, order->_marketOrderId, tickerId, order->_side, order->_price, orderQty, PRIORITY_INVALID};
            _matchingEngine->sendMarketUpdate(&_marketUpdate);
            removeOrder(order);
        }
        else
        {
            _marketUpdate = { MarketUpdateType::MODIFY, order->_marketOrderId, tickerId, order->_side, order->_price, orderQty, order->_priority};
            _matchingEngine->sendMarketUpdate(&_marketUpdate);
        }
    }


    auto OrderBook::checkForMatch(ClientId clientId, OrderId clientOrderId, TickerId tickerId, Side side, Price price, Qty qty, OrderId newMarketOrderId) noexcept
    {
        auto leftQty = qty;

        if (side == Side::BUY)
        {
            while (leftQty && _asksByPrice)
            {
                const auto askItr = _asksByPrice->_firstOrder;
                if (price < askItr->_price) [[ likely ]]
                {
                    break;
                }
                match(tickerId, clientId, side, clientOrderId, newMarketOrderId, askItr, &leftQty);
            }
        }
        else // SIDE == SELL
        {
            while (leftQty && _bidsByPrice)
            {
                const auto bidItr = _bidsByPrice->_firstOrder;
                if (price > bidItr->_price) [[ likely ]]
                {
                    break;
                }
                match(tickerId, clientId, side, clientOrderId, newMarketOrderId, bidItr, &leftQty);
            }
        }
        return leftQty;
    }


    auto OrderBook::add(ClientId clientId, OrderId clientOrderId, TickerId tickerId, Side side, Price price, Qty qty) noexcept -> void
    {
        using namespace ::common::enums;

        const auto newMarketOrderId = generateNewMarketOrderId();
        _clientResponse = {ClientResponseType::ACCEPTED, clientId, tickerId, clientOrderId, newMarketOrderId, side, price, 0, qty};
        _matchingEngine->sendClientResponse(&_clientResponse);

        const auto leftQty = checkForMatch(clientId, clientOrderId, tickerId, side, price, qty, newMarketOrderId);

        if (leftQty) [[ likely ]]
        {
            const auto priority = getNextPriority(price);
            auto order = _orderPool.allocate(tickerId, clientId, clientOrderId, newMarketOrderId, side, price, leftQty, priority, nullptr, nullptr);
            addOrder(order);
            _marketUpdate = {MarketUpdateType::ADD, newMarketOrderId, tickerId, side, price, leftQty, priority};
            _matchingEngine->sendMarketUpdate(&_marketUpdate);
        }
    }

    auto OrderBook::cancel(ClientId clientId, OrderId orderId, TickerId tickerId) noexcept -> void
    {
        using namespace ::common::enums;

        auto isCancelable       = (clientId < _cidOidToOrder.size());
        Order* exchangeOrder    = nullptr;
        
        if (isCancelable) [[ likely ]]
        {
            auto& coItr     = _cidOidToOrder.at(clientId);
            exchangeOrder   = coItr.at(orderId);
            isCancelable    = (exchangeOrder != nullptr);
        }
        if (!isCancelable) [[ unlikely ]]
        {
            _clientResponse = {ClientResponseType::CANCEL_REJECTED, clientId, tickerId, orderId,
                            ORDERID_INVALID, Side::INVALID, PRICE_INVALID, QTY_INVALID, QTY_INVALID};
        }
        else
        {
            _clientResponse = {ClientResponseType::CANCELED, clientId, tickerId, orderId,
                            exchangeOrder->_marketOrderId, exchangeOrder->_side, exchangeOrder->_price, QTY_INVALID, exchangeOrder->_qty};
            _marketUpdate   = {MarketUpdateType::CANCEL, exchangeOrder->_marketOrderId, tickerId, exchangeOrder->_side, exchangeOrder->_price, 0, exchangeOrder->_priority};
            removeOrder(exchangeOrder);
            _matchingEngine->sendMarketUpdate(&_marketUpdate);
        }
        _matchingEngine->sendClientResponse(&_clientResponse);
    }

    void OrderBook::removeOrder(Order* order) noexcept
    {
        auto ordersAtPrice = getOrdersAtPrice(order->_price);

        if (order->_prevOrder == order)
        {
            removeOrdersAtPrice(order->_side, order->_price);
        }
        else
        {
            const auto orderBefore  = order->_prevOrder;
            const auto orderAfter   = order->_nextOrder;
            orderBefore->_nextOrder = orderAfter;
            orderAfter->_prevOrder  = orderBefore;

            if (ordersAtPrice->_firstOrder == order)
            {
                ordersAtPrice->_firstOrder = orderAfter;
            }
            order->_prevOrder = order->_nextOrder = nullptr;
        }
        _cidOidToOrder.at(order->_clientId).at(order->_clientOrderId) = nullptr;
        _orderPool.deallocate(order);
    }

    void OrderBook::removeOrdersAtPrice(Side side, Price price) noexcept
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

        _ordersAtPrice.at(priceToIndex(price)) = nullptr;
        _ordersAtPricePool.deallocate(ordersAtPrice);

    }

    OrderId OrderBook::generateNewMarketOrderId() noexcept
    {
        return _nextMarketOrderId++;
    }

    auto OrderBook::getOrdersAtPrice(Price price) const noexcept -> OrdersAtPrice*
    {
        return _ordersAtPrice.at(priceToIndex(price));
    }
} // namespace exchange::matching_engine
