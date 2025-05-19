#pragma once
#include <string>
#include "OrderTickerSideHashMap.hpp"
#include "../../../common/types/types.hpp"
#include "../risk_management/RiskManager.hpp"

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

namespace trading::strategy
{
    class TradeEngine;
}

namespace trading::strategy::order_management
{
    struct Order;

    using namespace common::logging;
    using namespace common::types;
    using namespace common::messages;
    using namespace trading::strategy;
    namespace rm = risk_management;

    class OrderManager
    {

        public:
            OrderManager(Logger* logger, TradeEngine* tradeEngine, rm::RiskManager& riskManager);

            auto getOrderSideHashMap(TickerId tickerId) const;
            void moveOrders(TickerId tickerId, Price bidPrice, Price askPrice, Qty clip) noexcept;
            void onOrderUpdate(const ClientResponse* clientResponse) noexcept;
        private:
            void moveOrder(Order* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept;
            void newOrder(Order* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept;
            void cancelOrder(Order* order) noexcept;

            TradeEngine* _tradeEngine   = nullptr;
            const rm::RiskManager& _riskManager;
            std::string _timeStr;
            Logger* _logger             = nullptr;
            OrderTickerSideHashMap _tickerSideOrder;
            OrderId _nextOrderId        = 1;
    };
}