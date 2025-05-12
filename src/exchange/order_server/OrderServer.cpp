#include "exchange/order_server/OrderServer.hpp"
#include "common/messages/OMClientRequest.hpp"

namespace exchange::order_server
{
    OrderServer::OrderServer(ds::ClientRequestLFQueue* clientRequests, ds::ClientResponseLFQueue* clientResponses, const std::string& iface, int port)
    : _iface(iface), _port(port), _outgoingResp(clientResponses), _logger("exchange_order_server.log"), _tcpServer(_logger), _fifoSequencer(clientRequests, &_logger)
    {
        _cidNextOutgoingSeqNum.fill(1);
        _cidNextExpSeqNum.fill(1);
        _cidTcpSocket.fill(nullptr);
        _tcpServer.setRecvCallback([this] (auto socket, auto rxTime) { recvCallback(socket, rxTime); });
        _tcpServer.setRecvFinishedCallback([this] () { recvFinishedCallback(); });
    }

    auto OrderServer::start() -> void
    {
        _run = true;
        _tcpServer.listen(_iface, _port);
        common::assert(common::threading::createAndStartThread(-1, "Exchange/OrderServer", [this] () { run(); }) != nullptr, "Failed toStart OrderServer thread.");
    }

    void OrderServer::run() noexcept
    {
        _logger.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, time::getCurrentTimeStr(&_timeStr));

        while (_run)
        {
            _tcpServer.poll();
            _tcpServer.sendAndRecv();

            for (auto clientResponse = _outgoingResp->getNextToRead(); _outgoingResp->size() && clientResponse; clientResponse = _outgoingResp->getNextToRead())
            {
                auto& nextOutgoingSeqNum = _cidNextOutgoingSeqNum[clientResponse->_clientId];
                _logger.log("%:% %() % Processing cid:% seq:% %\n",
                            __FILE__, __LINE__, __FUNCTION__, time::getCurrentTimeStr(&_timeStr), clientResponse->_clientId, nextOutgoingSeqNum, clientResponse->toString());
            
                common::assert(_cidTcpSocket[clientResponse->_clientId] != nullptr, "Don't have a socket for client ID : " + std::to_string(clientResponse->_clientId));
                _cidTcpSocket[clientResponse->_clientId]->send(&nextOutgoingSeqNum, sizeof(nextOutgoingSeqNum));
                _cidTcpSocket[clientResponse->_clientId]->send(&clientResponse, sizeof(clientResponse));
                _outgoingResp->updateReadIndex();
                nextOutgoingSeqNum++;
            }
        }
        
    }

    OrderServer::~OrderServer()
    {
        stop();
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);
    }

    void OrderServer::stop()
    {
        _run = false;
    }

    void OrderServer::recvCallback(common::network::TCPSocket* socket, common::time::Nanos rxTime) noexcept
    {
        namespace msg = common::messages;
        _logger.log("%:% %() % Received socket:% len:% rx:%\n",
                    __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), socket->_fd, socket->_nextRcvValidIndex, rxTime);
        
        if (socket->_nextRcvValidIndex >= sizeof(msg::OMClientRequest))
        {
            size_t i = 0;
            for (; i + sizeof(msg::OMClientRequest) <= sizeof(msg::OMClientRequest); i += sizeof(msg::OMClientRequest))
            {
                auto request = reinterpret_cast<const msg::OMClientRequest *>(socket->_rcvBuffer + i);
                _logger.log("%:% %() % Received %\n",
                            __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr), request->toString());
            
                if (_cidTcpSocket[request->_clientRequest._clientId] == nullptr) [[ unlikely ]]
                {
                    _cidTcpSocket[request->_clientRequest._clientId] = socket;
                }
                
                if (_cidTcpSocket[request->_clientRequest._clientId] != socket)
                {
                    _logger.log("%:% %() % Received ClientRequest from ClientId:% on different socket:% expected:%\n",
                                __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr),
                                request->_clientRequest._clientId, socket->_fd, _cidTcpSocket[request->_clientRequest._clientId]->_fd);
                        continue;   
                }

                auto& nextExpSeqNum = _cidNextExpSeqNum[request->_clientRequest._clientId];
                
                if (request->_seqNum != nextExpSeqNum)
                {
                    _logger.log("%:% %() % Incorrect sequence number. ClientId:% SeqNum expected:% received:%\n",
                                __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&_timeStr),
                                request->_clientRequest._clientId, nextExpSeqNum, request->_seqNum);
                    continue;
                }
                ++nextExpSeqNum;
                _fifoSequencer.addClientRequest(rxTime, request->_clientRequest);
            }
            memcpy(socket->_rcvBuffer, socket->_rcvBuffer + i, socket->_nextRcvValidIndex - i);
            socket->_nextRcvValidIndex -= i;
        }
    }

    void OrderServer::recvFinishedCallback() noexcept
    {
        _fifoSequencer.sequenceAndPublish();
    }
} // namespace exchange::order_server