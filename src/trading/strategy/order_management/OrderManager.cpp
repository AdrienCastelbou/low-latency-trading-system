#pragma once
#include "common/messages/ClientRequest.hpp"
#include "common/enums/ClientRequestType.hpp"
#include "common/enums/ClientResponse.hpp"
#include "common/enums/ClientResponseType.hpp"
#include "common/time/time.hpp"

namespace trading::strategy::order_management
{
    using namespace common::types;
    using namespace common::logging;
    using namespace common::messages;
    using namespace common::enums;
    using namespace common::time;

    OrderManager::OrderManager(Logger* logger, TradeEngine* tradeEngine, RiskManager& riskManager)
    : _logger(logger), _tradeEngine(tradeEngine), _riskManager(riskManager)
    {
    }

    void OrderManager::moveOrders(TickerId tickerId, Price bidPrice, Price askPrice, Qty clip) noexcept
    {
        auto bidOrder = &(_tickerSideOrder.at(tickerId).at(sideToIndex(Side::BID)));
        moveOrder(bidOrder, tickerId, bidPrice, Side::BID, clip);
        auto askOrder = &(_tickerSideOrder.at(tickerId).at(sideToIndex(Side::ASK)));
        moveOrder(askOrder, tickerId, askPrice, Side::ASK, clip);
    }


    void OrderManager::moveOrder(Order* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept
    {
        switch (order->_orderState)
        {
            using enum OrderState;
            case LIVE:
            {
                if (order->_price != price || order->_qty != qty)
                {
                    cancelOrder(order);
                }
            }
            break;
            case INVALID:
            case DEAD:
            {
                if (price != PRICE_INVALID) [[ likely ]]
                {
                    const auto riskResult = _riskManager.checkPreTradeRisk(tickerId, side, qty);

                    if (riskResult == RiskCheckResult::ALLOWED) [[ likely ]]
                    {
                        newOrder(order, tickerId, price, side, qty);
                    }
                    else
                    {
                        _logger->log("%:% %() % Ticker: % Side: % Qty: % RiskCheckResult: %\n",
                                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr),
                                    tickerIdToString(tickerId), sideToStr(side), qtyToString(qty), riskCheckResultToString(riskResult));
                    } 
                }
            }
            break;
            case PENDING_NEW:
            case PENDING_CANCEL:
            break;
        }
    }

    void OrderManager::newOrder(Order* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept
    {
        const ClientRequest newRequest{ClientRequestType::NEW, _tradeEngine->clientId(), tickerId, _nextOrderId, side, price, qty};

        _tradeEngine->sendClientRequest(&newRequest);
        *order = {tickerId, _nextOrderId, side, price, qty, OrderState::PENDING_NEW};
        ++_nextOrderId;
        _logger->log("%:% %() % Sent new order % for %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr),
                    newRequest.toString().c_str(), order->toString().c_str());
    }

    void OrderManager::cancelOrder(Order* order) noexcept
    {
        const ClientRequest cancelRequest{ClientRequestType::CANCEL, _tradeEngine->clientId(), order->_tickerId, order->_orderId, order->_side, order->_price, order->_qty};
        
        _tradeEngine->sendClientRequest(&cancelRequest);
        order->_orderState = OrderState::PENDING_CANCEL;
        _logger->log("%:% %() % Sent cancel % for %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr),
                    cancelRequest.toString().c_str(), order->toString().c_str());
    }

    void OrderManager::onOrderUpdate(ClientResponse* clientResponse) noexcept
    {
        _logger->log("%:% %() % %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), clientResponse->toString().c_str());
        auto order = &(_tickerSideOrder.at(clientResponse->_tickerId).at(sideToIndex(clientResponse->_side)));
        _logger->log("%:% %() % %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), order->toString().c_str());

        switch (clientResponse->_type)
        {
            using enum ClientResponseType;
            case ACCEPTED:
            {
                order->_orderState = OrderState::LIVE;
            }
            break;
            case CANCELLED:
            {
                order->_orderState = OrderState::DEAD;
            }
            break;
            case FILLED:
            {
                order->_qty = clientResponse->_leavesQty;
                if (order->_qty == 0)
                {
                    order->_orderState = OrderState::DEAD;
                }
            }
            break;
            case CANCEL_REJECTED:
            case INVALID:
            break;
        }
    }

    auto OrderManager::getOrderSideHashMap(TickerId tickerId) const
    {
        return &(_tickerSideOrder.at(tickerId));
    }
}