#include "exchange/market_data/SnapshotSynthesizer.hpp"
#include "common/threading/threading.hpp"
#include "common/messages/MarketUpdate.hpp"

namespace exchange::market_data
{
    SnapshotSynthesizer::SnapshotSynthesizer(shared::MDPMarketUpdateLFQueue* marketUpdates, const std::string& iface, const std::string& snapshotIp, int snapshotPort)
    : _snapshotMdUpdates(marketUpdates), _logger("exchange_snapshot_synthesizer.log"), _snapshotSocket(_logger), _orderPool(shared::constants::MAX_ORDER_IDS)
    {
        assert(_snapshotSocket.init(snapshotIp, iface, snapshotPort, false) >= 0, "Unable to create snapshot mcast socket. error:" + std::string(std::strerror(errno)));
    }

    void SnapshotSynthesizer::start()
    {
        _run = true;
        assert(common::threading::createAndStartThread(-1, "Exchange/SnapshotSynthesizer", [this] () { run(); }) != nullptr, "Failed to start SnapshotSynthesizer thread");
    }

    void SnapshotSynthesizer::run() 
    {
        _logger.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr));
        while (_run)
        {
            for (auto marketUpdate = _snapshotMdUpdates->getNextToRead(); _snapshotMdUpdates->size() && marketUpdate; marketUpdate = _snapshotMdUpdates->getNextToRead())
            {
                _logger.log("%:% %() % Processing %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), marketUpdate->toString());
                addToSnapshot(marketUpdate);
                _snapshotMdUpdates->updateReadIndex();
            }
            if (common::time::getCurrentNanos() - _lastSnapshotTime > 60 * common::time::NANOS_TO_SECS)
            {
                _lastSnapshotTime = common::time::getCurrentNanos();
                publishSnapshot();
            }
        }
    }

    void SnapshotSynthesizer::addToSnapshot(const messages::MDPMarketUpdate* mdpMarketUpdate)
    {
        const auto& marketUpdate = mdpMarketUpdate->_marketUpdate;
        auto* orders = &_tickerOrders.at(marketUpdate._tickerId);

        switch(marketUpdate._type)
        {
            case enums::MarketUpdateType::ADD:
            {
                auto order = orders->at(marketUpdate._orderId);
                assert(order == nullptr, "Received:" + marketUpdate.toString() + " but order already exists:" + (order ? order->toString() : ""));
                orders->at(marketUpdate._orderId) = _orderPool.allocate(marketUpdate);
            }
                break;
            case enums::MarketUpdateType::MODIFY:
            {
                auto order = orders->at(marketUpdate._orderId);
                assert(order != nullptr, "Received:" + marketUpdate.toString() + " but order does not exist");
                assert(order->_orderId == marketUpdate._orderId, "Expecting existing order to match new one.");
                order->_qty = marketUpdate._qty;
                order->_price = marketUpdate._price;
            }
                break;
            case enums::MarketUpdateType::CANCEL:
            {
                auto order = orders->at(marketUpdate._orderId);
                assert(order != nullptr, "Received:" + marketUpdate.toString() + " but order does not exist");
                assert(order->_orderId == marketUpdate._orderId, "Expecting existing order to match new one.");
                assert(order->_side == marketUpdate._side, "Expecting existing order side to match new one.");
                _orderPool.deallocate(order);
                orders->at(marketUpdate._orderId) = nullptr;
            }
                break;
            case enums::MarketUpdateType::CLEAR:
            case enums::MarketUpdateType::SNAPSHOT_START:
            case enums::MarketUpdateType::SNAPSHOT_END:
            case enums::MarketUpdateType::TRADE:
            case enums::MarketUpdateType::INVALID:
                break;
        }
        assert(mdpMarketUpdate->_seqNum == _lastIncSeqNum + 1, "Expected incremental seq num to increase");
        _lastIncSeqNum = mdpMarketUpdate->_seqNum;
    }

    void SnapshotSynthesizer::publishSnapshot()
    {
        size_t snapshotSize = 0;
        const messages::MDPMarketUpdate startMarketUpdate{snapshotSize++, {enums::MarketUpdateType::SNAPSHOT_START, _lastIncSeqNum}};
        
        _logger.log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), startMarketUpdate.toString());
        _snapshotSocket.send(&startMarketUpdate, sizeof(messages::MDPMarketUpdate));

        for (size_t tickerId = 0; tickerId < _tickerOrders.size(); ++tickerId)
        {
            const auto& orders = _tickerOrders.at(tickerId);
            messages::MarketUpdate marketUpdate;
            marketUpdate._type = enums::MarketUpdateType::CLEAR;
            marketUpdate._tickerId = tickerId;
            const messages::MDPMarketUpdate clearMarketUpdate{snapshotSize++, marketUpdate};
            _logger.log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), clearMarketUpdate.toString());
            _snapshotSocket.send(&clearMarketUpdate, sizeof(messages::MDPMarketUpdate));

            for (const auto order : orders)
            {
                if (order)
                {
                    const messages::MDPMarketUpdate mdpMarketUpdate{snapshotSize++, *order};
                    _logger.log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), mdpMarketUpdate.toString());
                    _snapshotSocket.send(&mdpMarketUpdate, sizeof(messages::MDPMarketUpdate));
                    _snapshotSocket.sendAndRecv();
                }
            }
        }

        const messages::MDPMarketUpdate endMarketUpdate{snapshotSize++, {enums::MarketUpdateType::SNAPSHOT_END, _lastIncSeqNum}};
        _logger.log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), endMarketUpdate.toString());
        _snapshotSocket.send(&endMarketUpdate, sizeof(messages::MDPMarketUpdate));
        _snapshotSocket.sendAndRecv();
        _logger.log("%:% %() % Published snapshot of % orders\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), snapshotSize);
    }

    SnapshotSynthesizer::~SnapshotSynthesizer()
    {
        stop();
    }

    void SnapshotSynthesizer::stop()
    {
        _run = false;
    }
} // namespace exchange::market_data
