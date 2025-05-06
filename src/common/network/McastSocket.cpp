#include "common/network/McastSocket.hpp"
#include "common/logging/Logger.hpp"
#include "common/network/network.hpp"
#include <unistd.h>
#include "common/time/time.hpp"

namespace common::network
{
    constexpr size_t MCAST_BUFFER_SIZE = 64 * 1024 * 1024; // 64MB

    McastSocket::McastSocket(common::logging::Logger& logger)
        : _logger(logger)
    {
        _outboundData.resize(MCAST_BUFFER_SIZE);
        _inboundData.resize(MCAST_BUFFER_SIZE);
        
    }

    int McastSocket::init(const std::string& ip, const std::string& iface, int port, bool isListening)
    {
        _socketFd = common::network::createSocket(_logger, ip, iface, port, true, false, isListening, 0, false);
        return _socketFd;
    }

    bool McastSocket::join(const std::string& ip)
    {
        return common::network::join(_socketFd, ip);
    }

    void McastSocket::leave()
    {
        close(_socketFd);
        _socketFd = -1;
    }

    bool McastSocket::sendAndRecv() noexcept
    {
        const ssize_t nRcv = recv(_socketFd, _inboundData.data() + _nextRcvValidIndex, _inboundData.size() - _nextRcvValidIndex, MSG_DONTWAIT);

        if (nRcv > 0)
        {
            _nextRcvValidIndex += nRcv;
            _logger.log("%:% %() % read socket:% len :%\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), _socketFd, _nextRcvValidIndex);
            _recvCallback(this);
        }

        if (_nextSendValidIndex > 0)
        {
            ssize_t nSent = ::send(_socketFd, _outboundData.data(), _nextSendValidIndex, MSG_DONTWAIT | MSG_NOSIGNAL);
            _logger.log("%:% %() % send socket:% len :%\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), _socketFd, nSent);
        }
        _nextSendValidIndex = 0;
        return nRcv > 0;
    }

    void McastSocket::send(const void* data, size_t len) noexcept
    {
        memcpy(_outboundData.data() + _nextSendValidIndex, data, len);
        _nextSendValidIndex += len;
        common::assert(_nextSendValidIndex < MCAST_BUFFER_SIZE, "Mcast socket buffer filled up and sendAndRecv() not called");
    }
} // namespace common::network
