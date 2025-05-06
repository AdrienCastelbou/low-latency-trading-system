#pragma once

#include <functional>
#include <string>

namespace common::logging
{
    class Logger;
} // namespace common::logging

namespace common::network
{
    struct McastSocket
    {
        McastSocket(common::logging::Logger& logger);

        int init(const std::string& ip, const std::string& iface, int port, bool isListening);
        bool join(const std::string& ip);
        void leave();
        bool sendAndRecv() noexcept;
        void send(const void* data, size_t len) noexcept;

        int _socketFd = -1;

        std::vector<char> _outboundData;
        size_t _nextSendValidIndex = 0;
        std::vector<char> _inboundData;
        size_t _nextRcvValidIndex = 0;
        std::function<void(McastSocket*)> _recvCallback = nullptr;
        std::string _timeStr;
        common::logging::Logger& _logger;
    };
} // namespace common::network

