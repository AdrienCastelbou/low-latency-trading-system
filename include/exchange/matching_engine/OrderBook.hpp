#pragma once

#include "../../common/types/OrderTypes.hpp"
#include "../../common/messages/ClientResponse.hpp"
#include "../../common/messages/MarketUpdate.hpp"
#include "../../common/data_structures/MemoryPool.hpp"
#include "ClientOrderHashMap.hpp"
#include "OrdersAtPrice.hpp"
#include "OrdersAtPriceHashMap.hpp"

using namespace common::types;
using namespace common::data_structures;
using namespace common::messages;

namespace common::logging
{
    class Logger;
} // namespace common::logging


namespace exchange::matching_engine
{
    class MatchingEngine;
    
    class OrderBook final
    {
        public:
            OrderBook() = delete;
            OrderBook(TickerId tickerId, ::common::logging::Logger* logger, MatchingEngine* matchingEngine);
            OrderBook(const OrderBook&) = delete;
            OrderBook(const OrderBook&&) = delete;
            OrderBook& operator=(const OrderBook&) = delete;
            OrderBook& operator=(const OrderBook&&) = delete;
            ~OrderBook();

            auto add(ClientId clientId, OrderId clientOrderId, TickerId tickerId, Side side, Price price, Qty qty) noexcept -> void;
            auto cancel(ClientId clientId, OrderId orderId, TickerId tickerId) noexcept -> void;
        private:
            auto checkForMatch(ClientId clientId, OrderId clientOrderId, TickerId tickerId, Side side, Price price, Qty qty, OrderId newMarketOrderId) noexcept;
            auto match(TickerId tickerId, ClientId ClientId, Side side, OrderId clientOrderId, OrderId newMarketOrderId, Order* itr, Qty* leftQty) noexcept; 
            void removeOrder(Order* order) noexcept;
            void removeOrdersAtPrice(Side side, Price price) noexcept;
            OrderId generateNewMarketOrderId() noexcept;
            auto priceToIndex(Price price) const noexcept;
            auto getOrdersAtPrice(Price price) const noexcept -> OrdersAtPrice*;
            auto addOrder(Order* order) noexcept;
            auto addOrdersAtPrice(OrdersAtPrice* newOrdersAtPrice) noexcept;
            auto getNextPriority(Price price) noexcept;
            
            TickerId _tickerId                  = TICKERID_INVALID;
            MatchingEngine* _matchingEngine     = nullptr;
            ClientOrderHashMap _cidOidToOrder;
            MemoryPool<OrdersAtPrice> _ordersAtPricePool;
            OrdersAtPrice* _bidsByPrice         = nullptr;
            OrdersAtPrice* _asksByPrice         = nullptr;
            OrdersAtPriceHashMap _ordersAtPrice;
            MemoryPool<Order> _orderPool;
            ClientResponse _clientResponse;
            MarketUpdate _marketUpdate;
            OrderId _nextMarketOrderId          = 1;
            std::string _timeStr;
            ::common::logging::Logger* _logger   = nullptr;


    };
} // namespace exchange::matching_engine
