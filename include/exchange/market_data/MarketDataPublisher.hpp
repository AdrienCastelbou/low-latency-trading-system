#pragma once
#include <functional>
#include "../../common/data_structures/MarketUpdateLFQueue.hpp"
#include "../shared/MDPMarketUpdateLFQueue.hpp"
#include "../../common/logging/Logger.hpp"
#include "../../common/network/McastSocket.hpp"
#include "SnapshotSynthesizer.hpp"

namespace exchange::market_data
{
    using namespace exchange;
    using namespace common;

    class MarketDataPublisher
    {
        public:
            MarketDataPublisher(data_structures::MarketUpdateLFQueue* marketUpdates, const std::string& iface, const std::string& snapshotIp, int snapshotPort, const std::string& incrementalIp, int incrementalPort);
            ~MarketDataPublisher();

            void start();
            void stop();
            void run() noexcept;
        private:
            size_t _nextIncSeqNum                               = 1;
            data_structures::MarketUpdateLFQueue* _outgoingMdUpdates     = nullptr;
            shared::MDPMarketUpdateLFQueue _snapshotMdUpdates;
            volatile bool _run = false;
            std::string _timeStr;
            logging::Logger _logger;
            network::McastSocket _incrementalSocket;
            SnapshotSynthesizer* _snapshotSynthesizer = nullptr;
    };
    
} // namespace exchange::market_data
