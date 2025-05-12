#pragma once

#include <cstddef>
#include "../../common/data_structures/ClientRequestLFQueue.hpp"
#include "../../common/time/time.hpp"
#include "../../common/messages/ClientRequest.hpp"

namespace common::logging
{
    class Logger;
} // namespace common::loggin


namespace exchange::order_server
{
    using namespace exchange;
    using namespace common;

    constexpr size_t MAX_PRENDING_REQUESTS = 1024;

    class FIFOSequencer final
    {
        public:
            FIFOSequencer(data_structures::ClientRequestLFQueue* clientRequests, logging::Logger* logger);

            void addClientRequest(time::Nanos rxTime, const messages::ClientRequest& request);
            void sequenceAndPublish();
        private:
            struct RecvTimeClienRequest
            {
                time::Nanos _recvTime = 0;
                messages::ClientRequest _request;
        
                auto operator<(const RecvTimeClienRequest& rhs) const
                {
                    return _recvTime < rhs._recvTime;
                }
            };
            
            data_structures::ClientRequestLFQueue* _incomingRequests = nullptr;
            std::string _timeStr;
            logging::Logger* _logger;
            std::array<RecvTimeClienRequest, MAX_PRENDING_REQUESTS> _pendingClientRequests;
            size_t _pendingSize = 0;
    };
} // namespace exchange::order_server
