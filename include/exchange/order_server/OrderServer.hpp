#pragma once

#include <functional>
#include "../../common/threading/threading.hpp"
#include "../../common/network/TCPServer.hpp"
#include "../../common/data_structures/ClientResponseLFQueue.hpp"
#include "../../common/data_structures/ClientRequestLFQueue.hpp"
#include "../../common/constants/constants.hpp"
#include "../../common/logging/Logger.hpp"
#include "FIFOSequencer.hpp"

namespace exchange::order_server
{
    using namespace exchange;
    namespace ds = common::data_structures;

    class OrderServer final
    {
        public:
            OrderServer(ds::ClientRequestLFQueue* clientRequests, ds::ClientResponseLFQueue* clientResponses, const std::string& iface, int port);
            ~OrderServer();
            auto start()    -> void;
            auto stop()     -> void;
            void run() noexcept;
        private:
            void recvCallback(common::network::TCPSocket* socket, common::time::Nanos rxTime) noexcept;
            void recvFinishedCallback() noexcept;

            std::string _iface;
            const int _port = 0;
            ds::ClientResponseLFQueue* _outgoingResp = nullptr;
            volatile bool _run = false;
            std::string _timeStr;
            common::logging::Logger _logger;
            std::array<size_t, constants::MAX_NUM_CLIENTS> _cidNextOutgoingSeqNum;
            std::array<size_t, constants::MAX_NUM_CLIENTS> _cidNextExpSeqNum;
            std::array<common::network::TCPSocket*, constants::MAX_NUM_CLIENTS> _cidTcpSocket;
            common::network::TCPServer _tcpServer;
            FIFOSequencer _fifoSequencer;

    };
} // namespace exchange::order_server
