#include "common/network/network.hpp"
#include <netdb.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include "common/logging/Logger.hpp"
#include "common/time/time.hpp"

namespace common::network
{
    std::string getIfaceIP(const std::string& iface)
    {
        char buff[NI_MAXHOST] = {'\0'};
        ifaddrs* ifaddr = nullptr;

        if (getifaddrs(&ifaddr) != -1)
        {
            for (auto* ifa = ifaddr; ifa; ifa = ifa->ifa_next)
            {
                if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && iface == ifa->ifa_name)
                {
                    getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), buff, sizeof(buff),  NULL, 0, NI_NUMERICHOST);
                    break;
                }
            }
            freeifaddrs(ifaddr);
        }
        return buff;
    }

    bool setNonBlocking(int fd)
    {
        const auto flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
        {
            return false;
        }
        if (flags & O_NONBLOCK)
        {
            return true;
        }

        return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
    }

    bool setNoDelay(int fd)
    {
        int one = 1;
        return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<void *>(&one), sizeof(one)) != -1;
    }

    bool wouldBlock()
    {
        return (errno == EWOULDBLOCK || errno == EINPROGRESS);
    }

    bool setTTL(int fd, int ttl)
    {
        return setsockopt(fd, IPPROTO_IP, IP_TTL, reinterpret_cast<void* >(&ttl), sizeof(ttl)) != -1;
    }

    bool setMcastTTL(int fd, int mcastTtl)
    {
        return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void* >(&mcastTtl), sizeof(mcastTtl)) != -1;
    }

    bool setSOTimestamp(int fd)
    {
        int one = 1;
        return setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, reinterpret_cast<void *>(one), sizeof(one)) != -1;
    }

    int createSocket(common::logging::Logger& logger, const std::string& tIp, const std::string& iface,
                    int port, bool isUdp, bool isBlocking, bool isListening, int ttl, bool needsSOTimestamp)
    {
        std::string timeStr;
        const auto ip = tIp.empty() ? getIfaceIP(iface) : tIp;
        
        logger.log("%:% %() % ip:% iface: % port:% isUdp:% isBlocking;% isListening:% ttl:% SO_time:%\n",
                __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&timeStr), ip, iface, port, isUdp, isBlocking, isListening, ttl, needsSOTimestamp);
        
        addrinfo* result = nullptr;
        const auto rc = getAddrInfo(ip, port, isUdp, isListening, &result);
        if (rc)
        {
            logger.log("getaddrinfo() failed. error:% errno:%\n", gai_strerror(rc), strerror(errno));
            return -1;
        }

        int fd  = -1;
        int one = 1;

        for (auto* rp = result; rp; rp = rp->ai_next)
        {
            fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            
            if (fd == -1)
            {
                logger.log("socket() failed. Errno:%\n", strerror(errno));
                return -1;
            }
            if (!isBlocking && !setNonBlocking(fd))
            {
                logger.log("setNonBlocking() failed. Errno:%", strerror(errno));
                return -1;
            }
            if (!isUdp && !setNoDelay(fd))
            {
                logger.log("setNoDelay() failed. Errno:%", strerror(errno));
                return -1;
            }
            if (!isListening && connect(fd, rp->ai_addr, rp->ai_addrlen) == 1 && !wouldBlock())
            {
                logger.log("connect() failed. Errno:%", strerror(errno));
                return -1;           
            }
            if (isListening && setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char* >(&one), sizeof(one)) == -1)
            {
                logger.log("setsockopt() SO_REUSEADDR failed. Errno:%", strerror(errno));
                return -1;               
            }
            if (isListening && bind(fd, rp->ai_addr, rp->ai_addrlen) == -1)
            {
                logger.log("bind() failed. Errno:%", strerror(errno));
                return -1;              
            }
            if (!isUdp && isListening && listen(fd, MAX_TCP_SERVER_BACKLOG) == -1)
            {
                logger.log("listen() failed. Errno:%", strerror(errno));
                return -1;         
            }

            if (isUdp && ttl)
            {
                const bool isMulticast = atoi(ip.c_str()) & 0xe0;
                if (isMulticast && !setMcastTTL(fd, ttl))
                {
                    logger.log("setMcastTTL() failed. Errno:%", strerror(errno));
                    return -1;      
                }
                if (!isMulticast && !setTTL(fd, ttl))
                {
                    logger.log("setTTL() failed. Errno:%", strerror(errno));
                    return -1; 
                }
            }
            if (needsSOTimestamp && !setSOTimestamp(fd))
            {
                logger.log("setSOTimestamp() failed. Errno:%", strerror(errno));
                return -1;       
            }
        }
        if (result)
        {
            freeaddrinfo(result);
            return fd;
        }
        return -1;
    }
} // namespace common::network
