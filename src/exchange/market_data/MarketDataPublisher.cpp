#include "exchange/market_data/MarketDataPublisher.hpp"
#include "exchange/shared/constants/constants.hpp"
#include "common/threading/threading.hpp"
#include "common/time/time.hpp"

namespace exchange::market_data
{
    namespace cnsts = exchange::shared::constants;
    using namespace common::data_structures;

    MarketDataPublisher::MarketDataPublisher(MarketUpdateLFQueue* marketUpdates, const std::string& iface, const std::string& snapshotIp, int snapshotPort, const std::string& incrementalIp, int incrementalPort)
                                            : _outgoingMdUpdates(marketUpdates), _snapshotMdUpdates(cnsts::MAX_MARKET_UPDATES), _run(false), _logger("exchange_market_data_publisher.log"), _incrementalSocket(_logger)
    {
        assert(_incrementalSocket.init(incrementalIp, iface, incrementalPort, false) >= 0, "Unable to create incremental mcast socket. error:" + std::string(std::strerror(errno)));
        _snapshotSynthesizer = new SnapshotSynthesizer(&_snapshotMdUpdates, iface, snapshotIp, snapshotPort);
    }

    void MarketDataPublisher::start()
    {
        _run = true;
        assert(common::threading::createAndStartThread(-1, "Exchange/MarketDataPublisher", [this] () { run(); }) != nullptr, "Failed to start MarketData thread");
        _snapshotSynthesizer->start();
    }

    void MarketDataPublisher::run() noexcept
    {
        _logger.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr));
        while (_run)
        {
            for (auto marketUpdate = _outgoingMdUpdates->getNextToRead(); _outgoingMdUpdates->size() && marketUpdate; marketUpdate = _outgoingMdUpdates->getNextToRead())
            {
                _logger.log("%:% %() % Sending seq:% %\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), _nextIncSeqNum, marketUpdate->toString().c_str());
                
                _incrementalSocket.send(&_nextIncSeqNum, sizeof(_nextIncSeqNum));
                _incrementalSocket.send(marketUpdate, sizeof(MarketUpdate));
                _outgoingMdUpdates->updateReadIndex();
                auto nextWrite = _snapshotMdUpdates.getNextToWriteTo();
                nextWrite->_seqNum = _nextIncSeqNum;
                nextWrite->_marketUpdate = *marketUpdate;
                _snapshotMdUpdates.updateWriteIndex();
                _nextIncSeqNum++;
            }
            _incrementalSocket.sendAndRecv();
        }
    }
    void MarketDataPublisher::stop()
    {
        _run = false;
        _snapshotSynthesizer->stop();
    }

    MarketDataPublisher::~MarketDataPublisher()
    {
        stop();
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(5s);
        delete _snapshotSynthesizer;
        _snapshotSynthesizer = nullptr;
    }
} // namespace exchange/market_data
