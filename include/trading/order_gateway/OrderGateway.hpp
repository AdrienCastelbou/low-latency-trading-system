#pragma once

#include <functional>
#include <string>
#include "../../common/constants/constants.hpp"
#include "../../common/types/OrderTypes.hpp"
#include "../../common/data_structures/ClientRequestLFQueue.hpp"
#include "../../common/data_structures/ClientResponseLFQueue.hpp"
#include "../../common/logging/Logger.hpp"
#include "../../common/network/TCPSocket.hpp"
#include "common/time/time.hpp"

namespace trading::order_gateway
{
    using namespace common::types;
    using namespace common::data_structures;
    using namespace common::logging;
    using namespace common::network;
    using namespace common::time;

    class OrderGateway
    {
        public:
            OrderGateway(ClientId clientId, ClientRequestLFQueue* clientRequests, ClientResponseLFQueue* clientResponses, std::string& ip, const std::string& iface, int port);
            ~OrderGateway();

            void start();
            void run() noexcept;
            void stop();
            void recvCallback(TCPSocket* socket, Nanos rxTime) noexcept;
             
        private:
            const ClientId _clientId;
            std::string _ip;
            const std::string _iface;
            const int _port = 0;
            ClientRequestLFQueue* _outgoingRequests = nullptr;
            ClientResponseLFQueue* _incomingResponses = nullptr;
            volatile bool _run = false;
            std::string _timeStr;
            Logger _logger;
            size_t _nextOutgoingSeqNum = 1;
            size_t _nextExpSeqNum = 1;
            TCPSocket _tcpSocket;
    };
}