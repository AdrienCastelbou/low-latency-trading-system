#include "common/network/TCPServer.hpp"
#include "common/common.hpp"
#include <unistd.h>
#include "common/network/network.hpp"

namespace common::network
{
    TCPServer::TCPServer(common::logging::Logger& logger) : _listenerSocket(logger), _logger(logger)
    {
        _recvCallback           = [this] (auto socket, auto rxTime) { defaultRecvCallBack(socket, rxTime); };
        _recvFinishedCallback   = [this] () { defaultRecvFinishedCallback(); }; 
    }

    void TCPServer::defaultRecvCallBack(TCPSocket* socket, common::time::Nanos rxTime) noexcept
    {
        _logger.log("%:% %() % TCPServer::defaultRecvCallback() socket:% len:% rx:%\n"
                    __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr),
                    socket->_fd, socket->_nextRcvValidIndex, rxTime);
    }

    void TCPServer::defaultRecvFinishedCallback() noexcept
    {
        _logger.log("%:% %() % TCPServer::defaultRecvFinishedCallback()\n",
                    __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr));
    }

    void TCPServer::setRecvCallback(const std::function<void(TCPSocket* s, common::time::Nanos rxTime)>& callback) noexcept
    {
        _recvCallback = callback;
    }
    void TCPServer::setRecvFinishedCallback(const std::function<void()>& callback) noexcept
    {
        _recvFinishedCallback = callback;
    }

    void TCPServer::sendAndRecv() noexcept
    {
        auto recv = false;
        for (auto socket : _receiveSockets)
        {
            if (socket->sendAndRecv())
            {
                recv = true;
            }
        }

        if (recv == true)
        {
            _recvFinishedCallback();
        }
        for (auto socket : _sendSockets)
        {
            socket->sendAndRecv();
        }
    }

    void TCPServer::poll() noexcept
    {        
        for (auto socket : _disconnectedSockets)
        {
            del(socket);
        }
        struct kevent events[1 + _sockets.size()];
        const int n = kevent(_kqfd, nullptr, 0, events, 1 + _sockets.size(), nullptr);
        bool haveNewConn = false;

        for (int i = 0; i < n; i++)
        {
            auto event          = events[i];
            TCPSocket* socket   = reinterpret_cast<TCPSocket*>(event.udata);

            if (event.filter == EVFILT_READ)
            {
                if (socket == &_listenerSocket)
                {
                    _logger.log("%:% %() % kevent listener_socket:%\n",
                                __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), socket->_fd);
                    haveNewConn = true;
                    continue;
                }
                _logger.log("%:% %() % EVFILT_READ socket:%\n",
                            __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), socket->_fd);
                
                if(std::find(_receiveSockets.begin(), _receiveSockets.end(), socket) == _receiveSockets.end())
                {
                    _receiveSockets.push_back(socket);
                }
            }
            else if (event.filter == EVFILT_WRITE)
            {
                _logger.log("%:% %() % EVFILT_WRITE socket:%\n",
                            __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), socket->_fd);
                if(std::find(_sendSockets.begin(), _sendSockets.end(), socket) == _sendSockets.end())
                {
                    _sendSockets.push_back(socket);
                }
            }

            if (event.flags & (EV_ERROR | EV_EOF))
            {
                _logger.log("%:% %() % EPOLLERR socket:%\n",
                            __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), socket->_fd);
                if(std::find(_disconnectedSockets.begin(), _disconnectedSockets.end(), socket) == _disconnectedSockets.end())
                {
                    _disconnectedSockets.push_back(socket);
                }
            }
        }

        if (haveNewConn)
        {
            handleNewConnection();
        }
    }

    void TCPServer::handleNewConnection()
    {
        _logger.log("%:% %() % handleNewConnection\n",
                    __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr));
        sockaddr_storage addr;
        socklen_t addr_len = sizeof(addr);
        auto fd = accept(_listenerSocket._fd, reinterpret_cast<sockaddr *>(&addr), &addr_len);

        if (fd == -1)
        {
            return;
        }
        common::assert(setNonBlocking(fd) && setNoDelay(fd), "Failed to set non-blocking or no-delay on socket:" + std::to_string(fd));
        _logger.log("%:% %() % accepted socket:%\n",
                    __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), fd);

        auto* socket = new TCPSocket(_logger);
        socket->_fd = fd;
        socket->_recvCallback = _recvCallback;
        common::assert(kqueueAdd(socket), "kevent() failed. error:" + std::string(std::strerror(errno)));
        if(std::find(_sockets.begin(), _sockets.end(), socket) == _sockets.end())
        {
            _sockets.push_back(socket);
        }
        if(std::find(_receiveSockets.begin(), _receiveSockets.end(), socket) == _receiveSockets.end())
        {
            _receiveSockets.push_back(socket);
        }   
    }

    void TCPServer::listen(const std::string& iface, int port)
    {
        destroy();
        _kqfd = kqueue();
        
        common::assert(_kqfd >= 0, "kqueue() failed, error : " + std::string(std::strerror(errno)));
        common::assert(_listenerSocket.connect("", iface, port, true),
                        "Listener socket failed to connect. iface:" + iface + " port:" + std::to_string(port) + " error:" + std::string(std::strerror(errno)));
        common::assert(kqueueAdd(&_listenerSocket), "kevent() failed. error:" + std::string(std::strerror(errno)));

    }

    bool TCPServer::kqueueAdd(TCPSocket* socket)
    {
        struct kevent ev{};
        EV_SET(&ev, socket->_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, socket);
        return kevent(_kqfd, &ev, 1, nullptr, 0, nullptr) != -1;
    }

    void TCPServer::del(TCPSocket* socket)
    {
        kqueueDel(socket);
        _sockets.erase(std::remove(_sockets.begin(), _sockets.end(), socket), _sockets.end());
        _receiveSockets.erase(std::remove(_receiveSockets.begin(), _receiveSockets.end(), socket), _receiveSockets.end());
        _sendSockets.erase(std::remove(_sendSockets.begin(), _sendSockets.end(), socket), _sendSockets.end());
    }

    bool TCPServer::kqueueDel(TCPSocket* socket)
    {
        struct kevent ev;
        EV_SET(&ev, socket->_fd, EVFILT_READ, EV_DELETE, 0, 0, socket);
        return kevent(_kqfd, &ev, 1, nullptr, 0, nullptr) != -1;
    }

    TCPServer::~TCPServer()
    {
        destroy();
        std::for_each(_sockets.begin(), _sockets.end(), [](TCPSocket* socket) { delete socket; });
        _sockets.clear();
    }

    void TCPServer::destroy()
    {
        close(_kqfd);
        _kqfd = -1;
        _listenerSocket.destroy();
    }
} // namespace common::network
