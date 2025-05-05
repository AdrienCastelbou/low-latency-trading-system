#pragma once

#include <string>
#include "../shared/ClientRequestLFQueue.hpp"
#include "../shared/ClientResponseLFQueue.hpp"
#include "../shared/MarketUpdateLFQueue.hpp"
#include <array>
#include "OrderBookHashMap.hpp"

namespace common
{
    namespace messages
    {
        struct ClientRequest; 
    } // namespace network::messages
    
    namespace logging
    {
        class Logger;   
    }
} // namespace common::logging

using namespace exchange::shared;
using namespace common::messages;

namespace exchange::matching_engine
{
    struct Order;

    class MatchingEngine final
    {
        public:
            MatchingEngine() = delete;
            MatchingEngine(ClientRequestLFQueue* clientRequests, ClientResponseLFQueue* clientResponses, MarketUpdateLFQueue* marketUpdates, ::common::logging::Logger& logger);
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

            OrderBookHashMap _tikcerOrderBook;
            ClientRequestLFQueue* _incomingRequests         = nullptr;
            ClientResponseLFQueue* _outgoingOGWResponses    = nullptr;
            MarketUpdateLFQueue* _ougoingMDUpdates          = nullptr;
            volatile bool _run                              = false;
            std::string _timeStr;
            ::common::logging::Logger& _logger;

    };
} // namespace exchange::matching_engine
