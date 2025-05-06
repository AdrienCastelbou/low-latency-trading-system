#include "common/network/TCPSocket.hpp"
#include "common/logging/Logger.hpp"
#include <unistd.h>
#include "common/network/network.hpp"
#include <sys/socket.h>

namespace common::network
{    
    void TCPSocket::defaultRecvCallBack(TCPSocket* socket, common::time::Nanos rxTime) noexcept
    {
        _logger.log("%:% %() % TCPSocket::defaultRecvCallBack() socket:% len:% rx:%\n",
                    __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), socket->_fd, socket->_nextRcvValidIndex, rxTime);
    }

    TCPSocket::TCPSocket(common::logging::Logger& logger) : _logger(logger)
    {
        _sendBuffer     = new char[TCP_BUFFER_SIZE];
        _rcvBuffer      = new char[TCP_BUFFER_SIZE];
        _recvCallback   = [this] (auto socket, auto rxTime) {
            defaultRecvCallBack(socket, rxTime);
        };
    }


    int TCPSocket::connect(const std::string& ip, const std::string& iface, int port, bool isListening)
    {
        destroy();
        _fd = createSocket(_logger, ip, iface, port, false, false, isListening, 0, true);
        _inInAddr.sin_addr.s_addr = INADDR_ANY;
        _inInAddr.sin_port = htons(port);
        _inInAddr.sin_family = AF_INET;
        return _fd;
    }

    bool TCPSocket::sendAndRecv()
    {
        using namespace common::time;

        char ctrl[CMSG_SPACE(sizeof(struct timeval))];
        struct cmsghdr *cmsg = (struct cmsghdr *) &ctrl;
        struct iovec iov;
        iov.iov_base        = _rcvBuffer + _nextRcvValidIndex;
        iov.iov_len         = TCP_BUFFER_SIZE - _nextRcvValidIndex;
        msghdr msg;
        msg.msg_control     = ctrl;
        msg.msg_controllen  = sizeof(ctrl);
        msg.msg_name        = &_inInAddr;
        msg.msg_namelen     = sizeof(_inInAddr);
        msg.msg_iov         = &iov;
        msg.msg_iovlen      = 1;
        const auto nRcv = recvmsg(_fd, &msg, MSG_DONTWAIT);
        
        if (nRcv > 0)
        {
            _nextRcvValidIndex  += nRcv;
            Nanos kernelTime    = 0;
            struct timeval timeKernel;

            if (cmsg->cmsg_level == SOL_SOCKET &&
                cmsg->cmsg_type  == SCM_TIMESTAMP &&
                cmsg->cmsg_len   == CMSG_LEN(sizeof(timeKernel)))
            {
                memcpy(&timeKernel, CMSG_DATA(cmsg), sizeof(timeKernel));
                kernelTime = timeKernel.tv_sec * NANOS_TO_SECS + timeKernel.tv_usec * NANOS_TO_MICROS;
            }
            const auto userTime = getCurrentNanos();
            _logger.log("%:% %() % read socket:% len:% utime:% ktime:% diff:%\n",
                        __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), _fd, _nextRcvValidIndex, userTime, kernelTime, (userTime - kernelTime));
            _recvCallback(this, kernelTime);
        }

        ssize_t nSend = std::min(TCP_BUFFER_SIZE, _nextSendValidIndex);
        
        while (nSend > 0)
        {
            auto nSendThisMsg = std::min(static_cast<ssize_t>(_nextSendValidIndex), nSend);
            const int flags = MSG_DONTWAIT | MSG_NOSIGNAL | (nSendThisMsg < nSend ? MSG_HAVEMORE : 0);
            auto n = ::send(_fd, _sendBuffer, nSendThisMsg, flags);
            if (n < 0) [[ unlikely ]]
            {
                if (!wouldBlock())
                {
                    _sendDisconnected = true;
                    break;
                }
            }
            _logger.log("%:% %() % send socket:% len:%\n",
                        __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), _fd, n);
            nSend -= n;
            common::assert(n == nSendThisMsg, "Don't support partial send lengts yet");
        }
        _nextSendValidIndex = 0;
        return (nRcv > 0);
    }
    void TCPSocket::send(const void* data, size_t len) noexcept
    {
        if (len > 0)
        {
            memcpy(_sendBuffer + _nextSendValidIndex, data, len);
            _nextSendValidIndex += len;
        }
    }

    void TCPSocket::destroy() noexcept
    {
        close(_fd);
        _fd = -1;
    }

    TCPSocket::~TCPSocket()
    {
        destroy();
        delete[] _sendBuffer;
        _sendBuffer = nullptr;
        delete[] _rcvBuffer;
        _rcvBuffer = nullptr;
    }
} // namespace common::network
