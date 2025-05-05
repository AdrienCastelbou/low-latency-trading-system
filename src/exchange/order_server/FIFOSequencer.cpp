#include "exchange/order_server/FIFOSequencer.hpp"
#include "common/common.hpp"
#include "common/logging/Logger.hpp"

namespace exchange::order_server
{
    FIFOSequencer::FIFOSequencer(shared::ClientRequestLFQueue* clientRequest, logging::Logger* logger)
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
        _logger->log("%:% %() % Processing % requests.\n",
                    __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), _pendingSize);
            
        std::sort(_pendingClientRequests.begin(), _pendingClientRequests.begin() + _pendingSize);
        for (size_t i = 0; i < _pendingSize; i++)
        {
            const auto& clientRequest = _pendingClientRequests.at(i);
            _logger->log("%:% %() % Writing RX:% Req:% to FIFO.\n",
                        __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), clientRequest._recvTime, clientRequest._request.toString());
            auto* nextWrite = _incomingRequests->getNextToWriteTo();
            *nextWrite = std::move(clientRequest._request);
            _incomingRequests->updateWriteIndex();
        }
        _pendingSize = 0;
    }

} // namespace exchange::order_server
