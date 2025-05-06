#include "exchange/matching_engine/MatchingEngine.hpp"
#include "exchange/shared/ClientRequestLFQueue.hpp"
#include "exchange/shared/ClientResponseLFQueue.hpp"
#include "exchange/shared/MarketUpdateLFQueue.hpp"
#include "common/logging/Logger.hpp"
#include "common/time/time.hpp"
#include "common/common.hpp"
#include "common/threading/threading.hpp"
#include "common/enums/ClientRequestType.hpp"

using namespace exchange::shared;

namespace exchange::matching_engine
{
    MatchingEngine::MatchingEngine(ClientRequestLFQueue* clientRequests, ClientResponseLFQueue* clientResponses, MarketUpdateLFQueue* marketUpdates)
                                    : _incomingRequests(clientRequests), _outgoingOGWResponses(clientResponses), _ougoingMDUpdates(marketUpdates),
                                    _logger(("exchange_matching_engine.log"))
    {
        for (size_t i = 0; i < _tickerOrderBook.size(); i++)
        {
            _tickerOrderBook[i] = new OrderBook(i, &_logger, this);
        }
    }

    MatchingEngine::~MatchingEngine()
    {
        using namespace std::literals::chrono_literals;

        _run = false;
        std::this_thread::sleep_for(1s);
        _incomingRequests = nullptr;
        _outgoingOGWResponses = nullptr;
        _ougoingMDUpdates = nullptr;
        for (auto& orderBook : _tickerOrderBook)
        {
            (void) orderBook;
            delete orderBook;
            orderBook = nullptr;
        }
    }

    void MatchingEngine::start()
    {
        _run = true;
        ::common::assert(::common::threading::createAndStartThread(-1, "Exchange/MatchingEngine", [this]() {run();}) != nullptr, "Failed to start a new thread");
    }

    void MatchingEngine::stop()
    {
        _run = false;
    }

    void MatchingEngine::run() noexcept
    {
        _logger.log("%:% %() %\n", __FILE__, __LINE__,
                    __FUNCTION__, ::common::time::getCurrentTimeStr(&_timeStr));
        
        while (_run)
        {
            const auto clientRequest = _incomingRequests->getNextToRead();

            if (clientRequest) [[ likely]]
            {
                _logger.log("%:% %() % Processing %\n",
                            __FILE__, __LINE__, __FUNCTION__, ::common::time::getCurrentTimeStr(&_timeStr), clientRequest->toString());
                processClientRequest(clientRequest);
                _incomingRequests->updateReadIndex();
            }
        }
    }

    void MatchingEngine::processClientRequest(const ClientRequest* clientRequest) noexcept
    {
        using namespace ::common::enums;

        auto orderBook = _tickerOrderBook[clientRequest->_tickerId];
        switch (clientRequest->_type)
        {
            case ClientRequestType::NEW:
                orderBook->add(clientRequest->_clientId,
                            clientRequest->_orderId,
                            clientRequest->_tickerId,
                            clientRequest->_side,
                            clientRequest->_price,
                            clientRequest->_qty);
                break;
            case ClientRequestType::CANCEL:
                orderBook->cancel(clientRequest->_clientId, clientRequest->_orderId, clientRequest->_tickerId);
                break;
            default:
                ::common::fatal("Received invalid client request type: " + clientRequestTypeToString(clientRequest->_type));
                break;
        }
    }

    void MatchingEngine::sendClientResponse(const ClientResponse* clientResponse) noexcept
    {
        _logger.log("%:% %() % Sending %\n",
                    __FILE__, __LINE__, __FUNCTION__, ::common::time::getCurrentTimeStr(&_timeStr), clientResponse->toString());

        auto nextWrite = _outgoingOGWResponses->getNextToWriteTo();
        *nextWrite = std::move(*clientResponse);
        _outgoingOGWResponses->updateWriteIndex();
    }

    void MatchingEngine::sendMarketUpdate(const MarketUpdate* marketUpdate) noexcept
    {
        _logger.log("%:% %() % Sending %\n",
                    __FILE__, __LINE__, __FUNCTION__, ::common::time::getCurrentTimeStr(&_timeStr), marketUpdate->toString());

        auto nextWrite = _ougoingMDUpdates->getNextToWriteTo();
        *nextWrite = std::move(*marketUpdate);
        _ougoingMDUpdates->updateWriteIndex();
    }

} // namespace exchange::matching_engine
