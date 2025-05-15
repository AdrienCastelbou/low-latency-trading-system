#pragma once
#include <string>
#include "TradeEngineCfgHashMap.hpp"
#include "../../common/types/types.hpp"

namespace common {
    namespace messages {
        struct MarketUpdate;
        struct ClientResponse;
    }

    namespace logging {
        class Logger;
    }

}
namespace trading::strategy
{
    namespace order_management {
        class OrderManager;
        class OrderBook;
    }

    class FeatureEngine;
    class TradeEngine;

    using namespace common::logging;
    using namespace common::types;
    using namespace common::messages;

    class LiquidityTaker
    {
        public:
            LiquidityTaker(Logger* logger, TradeEngine* tradeEngine, const FeatureEngine* featureEngine, order_management::OrderManager* orderManager, const TradeEngineCfgHashMap& _tickerCfg);
        
            void onOrderBookUpdate(TickerId tickerId, Price price, Side side, const order_management::OrderBook* book) noexcept;
            void onTradeUpdate(const MarketUpdate* marketUpdate, const order_management::OrderBook* book) noexcept;
            void onOrderUpdate(const ClientResponse* clientResponse) noexcept;
        private:
            const FeatureEngine* _featureEngine = nullptr;
            const order_management::OrderManager* _orderManager = nullptr;
            std::string _timeStr;
            Logger* _logger = nullptr;
            TradeEngineCfgHashMap _tickerCfg;

    };
}