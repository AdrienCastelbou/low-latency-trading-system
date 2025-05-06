#pragma once
#include <stddef.h>
#include <netdb.h>
#include <functional>
#include "../time/time.hpp"

namespace common::logging
{
    class Logger;   
} // namespace common::logging


namespace common::network
{
    constexpr size_t TCP_BUFFER_SIZE = 64 * 1024 * 1024;

    struct TCPSocket
    {
        public:
            TCPSocket() = delete;
            TCPSocket(common::logging::Logger& logger);
            TCPSocket(const TCPSocket &) = delete;
            TCPSocket(const TCPSocket &&) = delete;
            TCPSocket &operator=(const TCPSocket &) = delete;
            TCPSocket &operator=(const TCPSocket &&) = delete;
            ~TCPSocket();

            int connect(const std::string& ip, const std::string& iface, int port, bool isListening);
            bool sendAndRecv();
            void send(const void* data, size_t len) noexcept;
            void destroy() noexcept;

            void defaultRecvCallBack(TCPSocket* socket, common::time::Nanos rxTime) noexcept;
            
            int _fd                     = -1;
            char* _sendBuffer           = nullptr;
            size_t _nextSendValidIndex  = 0;
            char* _rcvBuffer            = nullptr;
            size_t _nextRcvValidIndex   = 0;
            bool _sendDisconnected      = false;
            bool _recvDisconnected      = false;
            sockaddr_in _inInAddr;
            std::function<void(TCPSocket* s, common::time::Nanos rxTime)> _recvCallback;
            std::string _timeStr;
            common::logging::Logger& _logger;
    };
} // namespace common::network
