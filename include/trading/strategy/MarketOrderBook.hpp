#pragma once

#include <string>
#include "../../common/logging/Logger.hpp"
#include "../../common/types/types.hpp"
#include "../../common/data_structures/MemoryPool.hpp"
#include "MarketOrder.hpp"
#include "OrderHashMap.hpp"
#include "MarketOrdersAtPrice.hpp"
#include "OrdersAtPriceHashMap.hpp"
#include "BBO.hpp"

namespace common
{
    namespace messages
    {
        struct MarketUpdate;
    }
}

namespace trading::strategy
{
    using namespace common::types;
    using namespace common::data_structures;
    using namespace common::logging;
    using namespace common::messages;

    class TradeEngine;

    class MarketOrderBook final
    {
        public:
            MarketOrderBook(TickerId tickerId, Logger* tradeEngine);
            ~MarketOrderBook();

            void setTradeEngine(TradeEngine* tradeEngine);
            void onMarketUpdate(MarketUpdate* marketUpdate) noexcept;
            auto priceToIndex(Price price) const noexcept;
            auto getOrdersAtPrice(Price price) const noexcept -> MarketOrdersAtPrice*;

        private:
        void addOrder(MarketOrder* order) noexcept;
        auto addOrdersAtPrice(MarketOrdersAtPrice* newOrdersAtPrice) noexcept;
        void removeOrder(MarketOrder* order) noexcept;
        void removeOrdersAtPrice(Side side, Price price) noexcept;
        void updateBBO(bool updateBid, bool updateAsk) noexcept;

            const TickerId _tickerId;
            TradeEngine* _tradeEngine = nullptr;
            OrderHashMap _oidToOrder;
            MemoryPool<MarketOrder> _orderPool;
            MemoryPool<MarketOrdersAtPrice> _ordersAtPricePool;
            MarketOrdersAtPrice* _bidsByPrice = nullptr;
            MarketOrdersAtPrice* _asksByPrice = nullptr;
            OrdersAtPriceHashMap _priceOrdersAtPrice;
            BBO _bbo;
            std::string _timeStr;
            Logger* _logger = nullptr;
    };
}