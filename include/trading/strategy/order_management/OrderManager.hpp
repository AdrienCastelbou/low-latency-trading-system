#pragma once
#include <string>
#include "OrderTickerSideHashMap.hpp"
#include "../../../common/types/types.hpp"

namespace common
{
    namespace messages
    {
        struct ClientResponse;
    }

    namespace logging
    {
        class Logger;
    }
}

namespace trading::strategy::order_management
{
    class TradeEngine;
    class Order;

    class OrderManager
    {
        using namespace common::logging;
        using namespace common::types;
        using namespace common::messages;

        public:
            OrderManager(Logger* logger, TradeEngine* tradeEngine, RiskManager& riskManager);

            auto getOrderSideHashMap(TickerId tickerId) const;
            void moveOrders(TickerId tickerId, Price bidPrice, Price askPrice, Qty clip) noexcept;
            void onOrderUpdate(ClientResponse* clientResponse) noexcept;
        private:
            void moveOrder(Order* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept;
            void newOrder(Order* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept;
            void cancelOrder(Order* order) noexcept;

            TradeEngine* _tradeEngine   = nullptr;
            const RiskManager& _riskManager;
            std::string _timeStr;
            Logger* _logger             = nullptr;
            OrderTickerSideHashMap _tickerSideOrder;
            OrderId _nextOrderId        = 1;
    };
}