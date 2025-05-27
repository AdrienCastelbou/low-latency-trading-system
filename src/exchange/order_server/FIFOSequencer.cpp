#include "exchange/order_server/FIFOSequencer.hpp"
#include "common/common.hpp"
#include "common/logging/Logger.hpp"

namespace exchange::order_server
{
    namespace ds = common::data_structures;

    FIFOSequencer::FIFOSequencer(ds::ClientRequestLFQueue* clientRequest, logging::Logger* logger)
                                : _incomingRequests(clientRequest), _logger(logger)
    {
    }

    void FIFOSequencer::addClientRequest(time::Nanos rxTime, const messages::ClientRequest& request)
    {
        if (_pendingSize >= _pendingClientRequests.size())
        {
            common::fatal("Too many pending requests");
        }
        _pendingClientRequests.at(_pendingSize++) = (RecvTimeClienRequest{rxTime, request});
    }

    void FIFOSequencer::sequenceAndPublish()
    {
        if (!_pendingSize) [[ unlikely ]]
        {
            return ;
        }
#if !defined(NDEBUG)
        _logger->log("%:% %() % Processing % requests.\n",
                    __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), _pendingSize);
#endif

        std::sort(_pendingClientRequests.begin(), _pendingClientRequests.begin() + _pendingSize);
        for (size_t i = 0; i < _pendingSize; i++)
        {
            const auto& clientRequest = _pendingClientRequests.at(i);
#if !defined(NDEBUG)
            _logger->log("%:% %() % Writing RX:% Req:% to FIFO.\n",
                        __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), clientRequest._recvTime, clientRequest._request.toString());
#endif
            auto* nextWrite = _incomingRequests->getNextToWriteTo();
            *nextWrite = std::move(clientRequest._request);
            _incomingRequests->updateWriteIndex();
            TTT_MEASURE(T2_OrderServer_LFQueue_write, (*_logger));
        }
        _pendingSize = 0;
    }

} // namespace exchange::order_server
