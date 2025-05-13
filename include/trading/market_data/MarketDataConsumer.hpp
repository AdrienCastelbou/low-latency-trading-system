#pragma once
#include <functional>
#include <map>
#include "../../common/data_structures/MarketUpdateLFQueue.hpp"
#include "../../common/logging/Logger.hpp"
#include "../../common/network/McastSocket.hpp"
#include "../../common/types/types.hpp"

namespace common::messages
{
    struct MDPMarketUpdate;
}

namespace trading::market_data
{
    using namespace common;
    
    class MarketDataConsumer final
    {
        using queued_market_update_t = std::map<size_t, messages::MarketUpdate>;

        public:
            MarketDataConsumer(types::ClientId clientId, data_structures::MarketUpdateLFQueue* marketUpdates, const std::string& iface, const std::string& snapshotIp, int snapshotPort, const std::string& incrementalIp, int incrementalPort);
            ~MarketDataConsumer();

            void start();
            void run();
            void stop();
        private:
            void recvCallback(network::McastSocket* socket) noexcept;
            void startSnapshotSync();
            void checkSnapshotSync();
            void queueMessage(bool isSnapshot, const messages::MDPMarketUpdate* request);

            size_t _nextExpIncSeqNum = 1;
            data_structures::MarketUpdateLFQueue* _incomingMdUpdates = nullptr;
            volatile bool _run = false;
            std::string _timeStr;
            logging::Logger _logger;
            network::McastSocket _incrementalMcastSocket;
            network::McastSocket _snapshotMcastSocket;
            bool _inRecovery = false;
            const std::string _iface;
            const std::string _snapshotIp;
            const int _snapshotPort;
            queued_market_update_t _incrementalQueuedMsgs;
            queued_market_update_t _snapshotQueuedMsgs;

    };
}