#pragma once
#include <string>
#include <functional>
#include "../../common/logging/Logger.hpp"
#include "../../common/types/types.hpp"
#include "../../common/data_structures/ClientRequestLFQueue.hpp"
#include "../../common/data_structures/ClientResponseLFQueue.hpp"
#include "../../common/data_structures/MarketUpdateLFQueue.hpp"
#include "../../common/time/time.hpp"
#include "order_book/MarketOrderBook.hpp"
#include "order_book/MarketOrderBookHashMap.hpp"
#include "FeatureEngine.hpp"
#include "PositionKeeper.hpp"
#include "order_management/OrderManager.hpp"
#include "risk_management/RiskManager.hpp"
#include "MarketMaker.hpp"
#include "LiquidityTaker.hpp"
#include "AlgoType.hpp"
#include "TradeEngineCfgHashMap.hpp"

namespace trading::strategy
{
    using namespace common::types;
    using namespace common::data_structures;
    using namespace common::time;
    using namespace common::logging;
    namespace om = order_management;
    namespace rm = risk_management;
    namespace ob = order_book;

    class TradeEngine
    {
        public:
            TradeEngine(ClientId clientId, AlgoType algoType, const TradeEngineCfgHashMap& tickerCfg, ClientRequestLFQueue* clientRequests, ClientResponseLFQueue* clientResponses, MarketUpdateLFQueue* marketUpdates);
            ~TradeEngine();

            void start();
            void run() noexcept;
            void stop();
            void sendClientRequest(const ClientRequest* clientRequest) noexcept;
            void onOrderBookUpdate(TickerId tickerId, Price price, Side side, ob::MarketOrderBook* book) noexcept;
            void onTradeUpdate(const MarketUpdate* marketUpdate, ob::MarketOrderBook* book) noexcept;
            void onOrderUpdate(const ClientResponse* clientResponse) noexcept;
            ClientId getClientId() const noexcept;
            std::function<void(TickerId ticker_id, Price price, Side side, ob::MarketOrderBook *book)> _algoOnOrderBookUpdate;
            std::function<void(const MarketUpdate* market_update, ob::MarketOrderBook *book)> _algoOnTradeUpdate;
            std::function<void(const ClientResponse* client_response)> _algoOnOrderUpdate;

            void initLastEventTime() noexcept;
            Nanos silentSeconds() noexcept;
        private:
            void defaultAlgoOnOrderBookUpdate(TickerId tickerId, Price price, Side side, ob::MarketOrderBook *book) noexcept;
            void defaultAlgoOnTradeUpdate(const MarketUpdate* marketUpdate, ob::MarketOrderBook *book) noexcept;
            void defaultAlgoOnOrderUpdate(const ClientResponse* clientResponse) noexcept;

            const ClientId _clientId;
            order_book::MarketOrderBookHashMap _tickerOrderBook;
            ClientRequestLFQueue* _outgoingOgwRequests = nullptr;
            ClientResponseLFQueue* _incomingOgwResponses = nullptr;
            MarketUpdateLFQueue* _incomingMdUpdates = nullptr;
            Nanos _lastEventTime = 0;
            volatile bool _run = false;
            std::string _timeStr;
            Logger _logger;
            FeatureEngine _featureEngine;
            PositionKeeper _positionKeeper;
            om::OrderManager _orderManager;
            rm::RiskManager _riskManager;
            MarketMaker* _mmAlgo = nullptr;
            LiquidityTaker* _takerAlgo = nullptr;

    };
}