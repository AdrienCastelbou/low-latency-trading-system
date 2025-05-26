#include "trading/strategy/TradeEngine.hpp"
#include "common/common.hpp"
#include "common/threading/threading.hpp"

namespace trading::strategy
{
    TradeEngine::TradeEngine(ClientId clientId, AlgoType algoType, const TradeEngineCfgHashMap& tickerCfg, ClientRequestLFQueue* clientRequests, ClientResponseLFQueue* clientResponses, MarketUpdateLFQueue* marketUpdates)
    : _clientId(clientId), _outgoingOgwRequests(clientRequests), _incomingOgwResponses(clientResponses), _incomingMdUpdates(marketUpdates), _logger("tradeEngine_" + std::to_string(clientId) + ".log"), _featureEngine(&_logger), _positionKeeper(&_logger), _orderManager(&_logger, this, _riskManager), _riskManager(&_logger, &_positionKeeper, tickerCfg)
    {
        for (TickerId i = 0; i < _tickerOrderBook.size(); i++)
        {
            _tickerOrderBook[i] = new ob::MarketOrderBook(i, &_logger);
            _tickerOrderBook[i]->setTradeEngine(this);
        }

        _algoOnOrderBookUpdate = [this] (auto tickerId, auto price, auto side, auto book) { defaultAlgoOnOrderBookUpdate(tickerId, price, side, book); };
        _algoOnTradeUpdate = [this] (auto marketUpdate, auto book) { defaultAlgoOnTradeUpdate(marketUpdate, book); };
        _algoOnOrderUpdate = [this] (auto clientResponse) { defaultAlgoOnOrderUpdate(clientResponse); };
        
        if (algoType == AlgoType::MAKER)
        {
            _mmAlgo = new MarketMaker(&_logger, this, &_featureEngine, &_orderManager, tickerCfg);
        }
        else if (algoType == AlgoType::TAKER)
        {
            _takerAlgo = new LiquidityTaker(&_logger, this, &_featureEngine, &_orderManager, tickerCfg);
        }

        for (TickerId i = 0; i < _tickerOrderBook.size(); i++)
        {
            _logger.log("%:% %() % Initialized % Ticker:% %.\n",
                        __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), algoTypeToString(algoType), i, tickerCfg.at(i).toString());
        }
    }

    void TradeEngine::defaultAlgoOnOrderBookUpdate(TickerId tickerId, Price price, Side side, ob::MarketOrderBook *book) noexcept
    {
        (void) book;
        _logger.log("%:% %() % ticker:% price:% side:%\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), tickerId, priceToString(price).c_str(), sideToString(side).c_str());
    }

    void TradeEngine::defaultAlgoOnTradeUpdate(const MarketUpdate* marketUpdate, ob::MarketOrderBook *book) noexcept
    {
        (void) book;
        _logger.log("%:% %() % %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), marketUpdate->toString().c_str());
    }

    void TradeEngine::defaultAlgoOnOrderUpdate(const ClientResponse* client_response) noexcept
    {
        _logger.log("%:% %() % %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), client_response->toString().c_str());
    }

    void TradeEngine::start()
    {
        using namespace common;

        _run = true;
        assert(threading::createAndStartThread(-1, "Trading/TradeEngine", [this] { run(); }) != nullptr, "Failed to start TradeEngine thread");
    }

    void TradeEngine::run() noexcept
    {
        using namespace common;

        _logger.log("%:% %() %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr));
        while (_run)
        {
            for (auto clientResponse = _incomingOgwResponses->getNextToRead(); clientResponse; clientResponse = _incomingOgwResponses->getNextToRead())
            {
                TTT_MEASURE(T9t_TradeEngine_LFQueue_read, _logger);
                _logger.log("%:% %() % Processing %\n",
                            __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), clientResponse->toString().c_str());
                onOrderUpdate(clientResponse);
                _incomingOgwResponses->updateReadIndex();
                _lastEventTime = getCurrentNanos();
            }

            for (auto marketUpdate = _incomingMdUpdates->getNextToRead(); marketUpdate; marketUpdate = _incomingMdUpdates->getNextToRead())
            {
                TTT_MEASURE(T9_TradeEngine_LFQueue_read, _logger);
                _logger.log("%:% %() % Processing %\n",
                            __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), marketUpdate->toString().c_str());
                assert(marketUpdate->_tickerId < _tickerOrderBook.size(), "Unknown tickerId to update " + marketUpdate->toString());
                _tickerOrderBook[marketUpdate->_tickerId]->onMarketUpdate(marketUpdate);
                _incomingMdUpdates->updateReadIndex();
                _lastEventTime = getCurrentNanos();
            }
        }
    }

    void TradeEngine::onOrderBookUpdate(TickerId tickerId, Price price, Side side, ob::MarketOrderBook* book) noexcept
    {
        _logger.log("%:% %() % ticker:% price:% side:%\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), tickerId, priceToString(price).c_str(), sideToString(side).c_str());
        const auto bbo = book->getBBO();

        START_MEASURE(Trading_PositionKeeper_updateBBO);
        _positionKeeper.updateBBO(tickerId, bbo);
        END_MEASURE(Trading_PositionKeeper_updateBBO, _logger);
        
        START_MEASURE(Trading_FeatureEngine_onOrderBookUpdate);
        _featureEngine.onOrderBookUpdate(tickerId, price, side, book);
        END_MEASURE(Trading_FeatureEngine_onOrderBookUpdate, _logger);
        
        START_MEASURE(Trading_TradeEngine_algoOnOrderBookUpdate_);
        _algoOnOrderBookUpdate(tickerId, price, side, book);
        END_MEASURE(Trading_TradeEngine_algoOnOrderBookUpdate_, _logger);
    }


    void TradeEngine::onTradeUpdate(const MarketUpdate* marketUpdate, ob::MarketOrderBook* book) noexcept
    {
        _logger.log("%:% %() % %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), marketUpdate->toString().c_str());
        
        START_MEASURE(Trading_FeatureEngine_onTradeUpdate);
        _featureEngine.onTradeUpdate(marketUpdate, book);
        END_MEASURE(Trading_FeatureEngine_onTradeUpdate, _logger);
        
        START_MEASURE(Trading_TradeEngine_algoOnTradeUpdate_);
        _algoOnTradeUpdate(marketUpdate, book);
        END_MEASURE(Trading_TradeEngine_algoOnTradeUpdate_, _logger);
    }

    void TradeEngine::onOrderUpdate(const ClientResponse* clientResponse) noexcept
    {
        using namespace common::enums;

        _logger.log("%:% %() % %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), clientResponse->toString().c_str());
        
        if (clientResponse->_type == ClientResponseType::FILLED) [[ unlikely ]]
        {
            START_MEASURE(Trading_PositionKeeper_addFill);
            _positionKeeper.addFill(clientResponse);
            END_MEASURE(Trading_PositionKeeper_addFill, _logger);
        }
        START_MEASURE(Trading_TradeEngine_algoOnOrderUpdate_);
        _algoOnOrderUpdate(clientResponse);
        END_MEASURE(Trading_TradeEngine_algoOnOrderUpdate_, _logger);
    }

    void TradeEngine::sendClientRequest(const ClientRequest* clientRequest) noexcept
    {
        _logger.log("%:% %() % Sending %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), clientRequest->toString().c_str());
        auto nextWrite = _outgoingOgwRequests->getNextToWriteTo();
        *nextWrite = std::move(*clientRequest);
        _outgoingOgwRequests->updateWriteIndex();
        TTT_MEASURE(T10_TradeEngine_LFQueue_write, _logger);
    }

    void TradeEngine::initLastEventTime() noexcept
    {
        _lastEventTime = getCurrentNanos();
    }

    Nanos TradeEngine::silentSeconds() noexcept
    {
        return (getCurrentNanos() - _lastEventTime) / NANOS_TO_SECS;
    }

    ClientId TradeEngine::getClientId() const noexcept
    {
        return _clientId;
    }

    void TradeEngine::stop()
    {
        while (_incomingOgwResponses->size() || _incomingMdUpdates->size())
        {
            _logger.log("%:% %() % Sleeping till all updates are consumed ogw-size:% md-size:%\n",
                        __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), _incomingOgwResponses->size(), _incomingMdUpdates->size());
            using namespace std::literals::chrono_literals;
            std::this_thread::sleep_for(10ms);
        }

        _logger.log("%:% %() % POSITIONS\n%\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), _positionKeeper.toString());
        _run = false;
    }

    TradeEngine::~TradeEngine()
    {
        _run = false;
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);

        delete _mmAlgo;
        delete _takerAlgo;
        _mmAlgo = nullptr;
        _takerAlgo = nullptr;
        for (auto& orderBook : _tickerOrderBook)
        {
            delete orderBook;
            orderBook = nullptr;
        }
        _outgoingOgwRequests = nullptr;
        _incomingOgwResponses = nullptr;
        _incomingMdUpdates = nullptr;
    }
}