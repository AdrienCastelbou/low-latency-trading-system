#include "trading/market_data/MarketDataConsumer.hpp"
#include "common/threading/threading.hpp"
#include "common/time/time.hpp"
#include "common/messages/MDPMarketUpdate.hpp"

namespace trading::market_data
{
    using namespace common;
    
    MarketDataConsumer::MarketDataConsumer(types::ClientId clientId, data_structures::MarketUpdateLFQueue* marketUpdates, const std::string& iface, const std::string& snapshotIp, int snapshotPort, const std::string& incrementalIp, int incrementalPort)
    : _incomingMdUpdates(marketUpdates), _run(false), _logger("trading_market_data_consumer_" + std::to_string(clientId) + ".log"), _incrementalMcastSocket(_logger), _snapshotMcastSocket(_logger), _iface(iface), _snapshotIp(snapshotIp), _snapshotPort(snapshotPort)
    {
        auto recvCallback = [this] (auto socket) {this->recvCallback(socket);};
        _incrementalMcastSocket._recvCallback = recvCallback;

        assert(_incrementalMcastSocket.init(incrementalIp, iface, incrementalPort, true) >= 0, "Unable to create incremental mcast socket. error:" + std::string(std::strerror(errno)));
        assert(_incrementalMcastSocket.join(incrementalIp), "Join failed on:" + std::to_string(_incrementalMcastSocket._fd) + " error:" + std::string(std::strerror(errno)));
        
        _snapshotMcastSocket._recvCallback = recvCallback;
    }

    void MarketDataConsumer::recvCallback(common::network::McastSocket* socket) noexcept
    {
        const auto isSnapshot = socket->_fd == _snapshotMcastSocket._fd;

        if (isSnapshot && !_inRecovery) [[ unlikely ]]
        {
            _logger.log("%:% %() % WARN Not expecting snapshot message\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr));
            return;
        }

        if (socket->_nextRcvValidIndex >= sizeof(messages::MDPMarketUpdate))
        {
            size_t i = 0;
            for (; i + sizeof(messages::MDPMarketUpdate) <= socket->_nextRcvValidIndex; i += sizeof(messages::MDPMarketUpdate))
            {
                auto request = reinterpret_cast<messages::MDPMarketUpdate*>(socket->_inboundData.data() + i);
                _logger.log("%:% %() % Received % socket len:% %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), (isSnapshot ? "snapshot" : "incremental"), sizeof(messages::MDPMarketUpdate), request->toString());
            
                const bool alreadyInRecovery = _inRecovery;
                _inRecovery = (alreadyInRecovery || request->_seqNum != _nextExpIncSeqNum);
           
                if (_inRecovery) [[ unlikely ]]
                {
                    if (!alreadyInRecovery) [[ unlikely ]]
                    {
                        _logger.log("%:% %() % Packet drops on % socket. SeqNum expected: %, received: %\n",
                                    __FILE__, __LINE__, __FUNCTION__, time::getCurrentTimeStr(&_timeStr), (isSnapshot? "snapshot" : "incremental"), _nextExpIncSeqNum, request->_seqNum);
                        startSnapshotSync();
                    }
                    queueMessage(isSnapshot, request);
                }
                else if (!isSnapshot)
                {
                    _logger.log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, time::getCurrentTimeStr(&_timeStr), request->toString());
                    ++_nextExpIncSeqNum;
                    auto nextWrite = _incomingMdUpdates->getNextToWriteTo();
                    *nextWrite = std::move(request->_marketUpdate);
                    _incomingMdUpdates->updateWriteIndex();
                }

                memcpy(socket->_inboundData.data(), socket->_inboundData.data() + i, socket->_nextRcvValidIndex - i);

                socket->_nextRcvValidIndex -= i;
            }
        }
    }

    void MarketDataConsumer::startSnapshotSync()
    {
        _snapshotQueuedMsgs.clear();
        _incrementalQueuedMsgs.clear();

        assert(_snapshotMcastSocket.init(_snapshotIp, _iface, _snapshotPort, true) >= 0, "Unable to create snapshot mcast socket. error:" + std::string(std::strerror(errno)));
        assert(_snapshotMcastSocket.join(_snapshotIp), "Join failed on:" + std::to_string(_snapshotMcastSocket._fd) + " error:" + std::string(std::strerror(errno)));
    }

    void MarketDataConsumer::queueMessage(bool isSnapshot, const messages::MDPMarketUpdate* request)
    {
        if (isSnapshot)
        {
            if (_snapshotQueuedMsgs.contains(request->_seqNum))
            {
                _logger.log("%:% %() % Packet drops on snapshot socket. Received for a 2nd time:%\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), request->toString());
                _snapshotQueuedMsgs.clear();
            }
            _snapshotQueuedMsgs[request->_seqNum] = request->_marketUpdate;
        }
        else
        {
            _incrementalQueuedMsgs[request->_seqNum] = request->_marketUpdate;
        }
        _logger.log("%:% %() % size snapshot:% incremental:% % => %\n", 
            __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), _snapshotQueuedMsgs.size(), _incrementalQueuedMsgs.size(), request->_seqNum, request->toString());
        checkSnapshotSync();
    }

    void MarketDataConsumer::checkSnapshotSync()
    {
        if (_snapshotQueuedMsgs.empty())
        {
            return;
        }
        const auto& firstSnapshotMsg = _snapshotQueuedMsgs.begin()->second;
        if (firstSnapshotMsg._type != enums::MarketUpdateType::SNAPSHOT_START)
        {
            _logger.log("%:% %() % Returning because have not seen a SNAPSHOT_START yet:\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr));
            _snapshotQueuedMsgs.clear();
            return;
        }

        std::vector<messages::MarketUpdate> finalEvents;
        auto haveCompletedSnapshot = true;
        size_t nextSnapshotSeq = 0;

        for (auto& snapshotMsg : _snapshotQueuedMsgs)
        {
            _logger.log("%:% %() % % => %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), snapshotMsg.first, snapshotMsg.second.toString());
            if (snapshotMsg.first != nextSnapshotSeq)
            {
                haveCompletedSnapshot = false;
                _logger.log("%:% %() % Detected gap in snapshot stream, expected:% received:%\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), nextSnapshotSeq, snapshotMsg.first);
                break;
            }
            if (snapshotMsg.second._type != enums::MarketUpdateType::SNAPSHOT_START && snapshotMsg.second._type != enums::MarketUpdateType::SNAPSHOT_END)
            {
                finalEvents.push_back(snapshotMsg.second);
            }
            ++nextSnapshotSeq;
        }

        if (!haveCompletedSnapshot)
        {
            _logger.log("%:% %() % Returning because found gaps in snapshot stream\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr));
            _snapshotQueuedMsgs.clear();
            return;
        }

        const auto& lastSnapshotMsg = _snapshotQueuedMsgs.rbegin()->second;
        if (lastSnapshotMsg._type != enums::MarketUpdateType::SNAPSHOT_END)
        {
            _logger.log("%:% %() % Returning because have not seen a SNAPSHOT_END yet:\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr));
            return;
        }

        haveCompletedSnapshot = true;
        size_t numIncrementals = 0;
        _nextExpIncSeqNum = lastSnapshotMsg._orderId + 1;

        for (auto& incrementalMsg : _incrementalQueuedMsgs)
        {
            _logger.log("%:% %() % Checking next exp: % vs seq:% %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), _nextExpIncSeqNum, incrementalMsg.first, incrementalMsg.second.toString());
            if (incrementalMsg.first < _nextExpIncSeqNum)
            {
                continue;
            }
            if (incrementalMsg.first != _nextExpIncSeqNum)
            {
                _logger.log("%:% %() % Detected gap in incremental stream, expected:% found:%\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), _nextExpIncSeqNum, incrementalMsg.first);
                haveCompletedSnapshot = false;
                break;
            }
            _logger.log("%:% %() % % => %\n", __FILE__, __LINE__, __FUNCTION__, incrementalMsg.first, incrementalMsg.second.toString());
            if (incrementalMsg.second._type!= enums::MarketUpdateType::SNAPSHOT_START && incrementalMsg.second._type!= enums::MarketUpdateType::SNAPSHOT_END)
            {
                finalEvents.push_back(incrementalMsg.second);
                ++_nextExpIncSeqNum;
                ++numIncrementals;
            }
        }

        if (!haveCompletedSnapshot)
        {
            _logger.log("%:% %() % Returning because found gaps in incremental stream\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr));
            _snapshotQueuedMsgs.clear();
            return;
        }

        for (const auto& event : finalEvents)
        {
            auto nextWrite = _incomingMdUpdates->getNextToWriteTo();
            *nextWrite = event;
            _incomingMdUpdates->updateWriteIndex();
        }
        _logger.log("%:% %() % Recovered % snapshot and % incremental orders\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), _snapshotQueuedMsgs.size() - 2, numIncrementals);
        _incrementalQueuedMsgs.clear();
        _snapshotQueuedMsgs.clear();
        _inRecovery = false;
        _snapshotMcastSocket.leave();
    }

    void MarketDataConsumer::start()
    {
        _run = true;
        assert(common::threading::createAndStartThread(-1, "Trading/MarketDataConsumer", [this] () { run(); })!= nullptr, "Failed to start MarketDataConsumer thread");
    }

    void MarketDataConsumer::run()
    {
        _logger.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr));
        while (_run)
        {
            _incrementalMcastSocket.sendAndRecv();
            _snapshotMcastSocket.sendAndRecv();
        }
    }

    MarketDataConsumer::~MarketDataConsumer()
    {
        stop();
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(5s);
    }

    void MarketDataConsumer::stop()
    {
        _run = false;
    }
}