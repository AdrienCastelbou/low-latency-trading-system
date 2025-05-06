#pragma once
#include "../../common/logging/Logger.hpp"
#include "../shared/MDPMarketUpdateLFQueue.hpp"
#include "../../common/network/McastSocket.hpp"
#include "../../common/messages/MarketUpdate.hpp"
#include "../shared/constants/constants.hpp"
#include "../../common/time/time.hpp"
#include "../../common/data_structures/MemoryPool.hpp"

namespace exchange::market_data
{
    using namespace common;
    using namespace exchange;
    
    class SnapshotSynthesizer final
    {
        public:
            SnapshotSynthesizer(shared::MDPMarketUpdateLFQueue* marketUpdates, const std::string& iface, const std::string& snapshotIp, int snapshotPort);
            ~SnapshotSynthesizer();
            
            void start();
            void stop();
            void addToSnapshot(const messages::MDPMarketUpdate* mdpMarketUpdate);
            void publishSnapshot();
            void run();
        private:
            shared::MDPMarketUpdateLFQueue* _snapshotMdUpdates = nullptr;
            common::logging::Logger _logger;
            volatile bool _run = false;
            std::string _timeStr;
            network::McastSocket _snapshotSocket;
            std::array<std::array<messages::MarketUpdate*, shared::constants::MAX_ORDER_IDS>, shared::constants::MAX_TICKERS> _tickerOrders;
            size_t _lastIncSeqNum = 0;
            time::Nanos _lastSnapshotTime = 0;
            data_structures::MemoryPool<messages::MarketUpdate> _orderPool;
    };
} // namespace exchange::market_data
