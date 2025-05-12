#pragma once

#include <string>
#include <array>
#include "../../common/data_structures/ClientRequestLFQueue.hpp"
#include "../../common/data_structures/ClientResponseLFQueue.hpp"
#include "../../common/data_structures/MarketUpdateLFQueue.hpp"
#include "OrderBookHashMap.hpp"
#include "../../common/logging/Logger.hpp"

namespace common
{
    namespace messages
    {
        struct ClientRequest; 
    } // namespace network::messages
} // namespace common::logging

using namespace common::messages;
using namespace common::data_structures;

namespace exchange::matching_engine
{
    struct Order;

    class MatchingEngine final
    {
        public:
            MatchingEngine() = delete;
            MatchingEngine(ClientRequestLFQueue* clientRequests, ClientResponseLFQueue* clientResponses, MarketUpdateLFQueue* marketUpdates);
            ~MatchingEngine();
            MatchingEngine(const MatchingEngine &)  = delete;
            MatchingEngine(const MatchingEngine &&) = delete;
            MatchingEngine &operator=(const MatchingEngine &)   = delete;
            MatchingEngine &operator=(const MatchingEngine &&)  = delete;

            void start();
            void stop();
            void sendClientResponse(const ClientResponse* clientResponse) noexcept;
            void sendMarketUpdate(const MarketUpdate* MarketUpdate) noexcept;

        private:
            void run() noexcept;
            void processClientRequest(const ClientRequest* clientRequest) noexcept;

            OrderBookHashMap _tickerOrderBook;
            ClientRequestLFQueue* _incomingRequests         = nullptr;
            ClientResponseLFQueue* _outgoingOGWResponses    = nullptr;
            MarketUpdateLFQueue* _ougoingMDUpdates          = nullptr;
            volatile bool _run                              = false;
            std::string _timeStr;
            common::logging::Logger _logger;

    };
} // namespace exchange::matching_engine
