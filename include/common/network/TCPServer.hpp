#pragma once

#include "TCPSocket.hpp"
#include <sys/event.h>
#include <vector>
#include <functional>
#include "../time/time.hpp"
#include "../logging/Logger.hpp"

namespace common::network
{

    class TCPServer final
    {
        public:
            TCPServer() = delete;
            TCPServer(common::logging::Logger& logger);
            TCPServer(const TCPServer &) = delete;
            TCPServer(const TCPServer &&) = delete;
            TCPServer &operator=(const TCPServer &) = delete;
            TCPServer &operator=(const TCPServer &&) = delete;

            void sendAndRecv() noexcept;
            void poll() noexcept;
            void listen(const std::string& iface, int port);
            void del(TCPSocket* socket);
            void destroy();

            void setRecvCallback(const std::function<void(TCPSocket* s, common::time::Nanos rxTime)>& callback) noexcept;
            void setRecvFinishedCallback(const std::function<void()>& callback) noexcept;
        private:
            void handleNewConnection();
            bool kqueueAdd(TCPSocket* socket);
            bool kqueueDel(TCPSocket* socket);

            int _kqfd = -1;
            TCPSocket _listenerSocket;
            //epoll_event _events[1024];
            std::vector<TCPSocket* > _sockets;
            std::vector<TCPSocket* > _receiveSockets;
            std::vector<TCPSocket* > _sendSockets;
            std::vector<TCPSocket* > _disconnectedSockets;
            std::function<void(TCPSocket* s, common::time::Nanos rxTime)> _recvCallback; 
            std::function<void()> _recvFinishedCallback;
            std::string _timeStr;
            common::logging::Logger &_logger;

            void defaultRecvCallBack(TCPSocket* socket, common::time::Nanos rxTime) noexcept;
            void defaultRecvFinishedCallback() noexcept;

    };
} // namespace common::network
