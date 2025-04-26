#pragma once
#include <string>
#include <ifaddrs.h>
#include <netdb.h>

namespace common::logging
{
    class Logger;
} // namespace common::logging


namespace common::network
{
    constexpr int MAX_TCP_SERVER_BACKLOG = 1024;

    std::string getIfaceIP(const std::string& iface);
    bool setNonBlocking(int fd);
    bool setNoDelay(int fd);
    bool setSOTimestamp(int fd);
    bool wouldBlock();
    bool setMcastTTL(int fd, int ttl);
    bool setTTL(int fd, int ttl);
    int join(int fd, const std::string& tIp, const std::string& iface, int port, bool isUDP, bool isBlocking, bool isListening, int ttl, bool needsSOTimestamp);

    int createSocket(common::logging::Logger& logger, const std::string& tIp, const std::string& iface,
                    int port, bool isUdp, bool isBlocking, bool isListening, int ttl, bool needsSOTimestamp);
    
    inline int getAddrInfo(const std::string& ip, int port, bool isUdp, bool isListening, addrinfo** result)
    {
        addrinfo hints{};

        hints.ai_family     = AF_INET;
        hints.ai_socktype   = isUdp ? SOCK_DGRAM : SOCK_STREAM;
        hints.ai_protocol   = isUdp ? IPPROTO_UDP : IPPROTO_TCP;
        hints.ai_flags      = isListening ? AI_PASSIVE : 0;
        if (std::isdigit(ip.c_str()[0]))
        {
            hints.ai_flags |= AI_NUMERICHOST;
        }
        hints.ai_flags |= AI_NUMERICSERV;

        return getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, result); 
    }
    
} // namespace common::network
