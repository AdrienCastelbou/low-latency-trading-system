#include "trading/order_gateway/OrderGateway.hpp"
#include "common/common.hpp"
#include "common/threading/threading.hpp"
#include "common/messages/OMClientResponse.hpp"
#include "common/messages/ClientRequest.hpp"

namespace trading::order_gateway
{
    OrderGateway::OrderGateway(ClientId clientId, ClientRequestLFQueue* clientRequests, ClientResponseLFQueue* clientResponses, std::string& ip, const std::string& iface, int port)
    : _clientId(clientId), _ip(ip), _iface(iface), _port(port), _outgoingRequests(clientRequests), _incomingResponses(clientResponses), _logger("trading_order_gateway" + std::to_string(clientId) + ".log"), _tcpSocket(_logger)
    {
        _tcpSocket._recvCallback = [this] (auto socket, auto rxTime) { recvCallback(socket, rxTime); };
    }

    void OrderGateway::start()
    {
        using namespace common::threading;

        _run = true;
        common::assert(_tcpSocket.connect(_ip, _iface, _port, false) >= 0, "Unable to connect to ip:" + _ip + " port:" + std::to_string(_port) + " on iface:" + _iface + " error:" + std::string(std::strerror(errno)));
        common::assert(createAndStartThread(-1, "Trading/OrderGateway", [this] () { run(); }) != nullptr, "Failed to start OrderGateway thread");
    }

    void OrderGateway::stop()
    {
        _run = false;
    }
    void OrderGateway::run() noexcept
    {
        using namespace common::time;
        using namespace common::messages;
        
        _logger.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr));
        while(_run)
        {
            _tcpSocket.sendAndRecv();

            for (auto clientRequest = _outgoingRequests->getNextToRead(); clientRequest; clientRequest = _outgoingRequests->getNextToRead())
            {
                _logger.log("%:% %() % sending cid:% seq:% %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), _clientId, _nextOutgoingSeqNum, clientRequest->toString());
                _tcpSocket.send(&_nextOutgoingSeqNum, sizeof(_nextOutgoingSeqNum));
                _tcpSocket.send(clientRequest, sizeof(ClientRequest));
                _outgoingRequests->updateReadIndex();
                _nextOutgoingSeqNum++;
            }
        }
    }

    void OrderGateway::recvCallback(TCPSocket* socket, Nanos rxTime) noexcept
    {
        using namespace common::messages;

        _logger.log("%:% %() % Received socket:% len:% %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), socket->_fd, socket->_nextRcvValidIndex, rxTime);

        if (socket->_nextRcvValidIndex >= sizeof(OMClientResponse))
        {
            size_t i = 0;
            
            for (; i + sizeof(OMClientResponse) <= socket->_nextRcvValidIndex; i += sizeof(OMClientResponse))
            {
                auto response = reinterpret_cast<const OMClientResponse*>(socket->_rcvBuffer + i);
                _logger.log("%:% %() % Received %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), response->toString());
                
                if (response->_clientResponse._clientId != _clientId)
                {
                    _logger.log("%:% %() % ERROR Incorrect client id. ClientId expected:% received:%\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), _clientId, response->_clientResponse._clientId);
                    continue;
                }
                if (response->_seqNum != _nextExpSeqNum)
                {
                    _logger.log("%:% %() % ERROR Incorrect sequence number. ClientId:%. SeqNum expected:% received:%\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), _clientId, _nextExpSeqNum, response->_seqNum);
                    continue;
                }
                _nextExpSeqNum++;
                auto nextWrite = _incomingResponses->getNextToWriteTo();
                *nextWrite = std::move(response->_clientResponse);
                _incomingResponses->updateWriteIndex();
            }
            memcpy(socket->_rcvBuffer, socket->_rcvBuffer + i, socket->_nextRcvValidIndex - i);
            socket->_nextRcvValidIndex -= i;
        }
    }


    OrderGateway::~OrderGateway()
    {
        _run = false;
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(5s);
    }
}