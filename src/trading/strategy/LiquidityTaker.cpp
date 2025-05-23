#include "trading/strategy/LiquidityTaker.hpp"
#include "common/logging/Logger.hpp"
#include "common/time/time.hpp"
#include "common/messages/MarketUpdate.hpp"
#include "trading/strategy/FeatureEngine.hpp"
#include "trading/strategy/TradeEngine.hpp"
#include "trading/strategy/order_book/MarketOrderBook.hpp"

namespace trading::strategy
{
    namespace om = order_management;
    using namespace common::logging;
    using namespace common::time;
    using namespace common::types;
    using namespace common::messages;

    LiquidityTaker::LiquidityTaker(Logger* logger, TradeEngine* tradeEngine, const FeatureEngine* featureEngine, om::OrderManager* orderManager, const TradeEngineCfgHashMap& _tickerCfg)
    : _featureEngine(featureEngine), _orderManager(orderManager), _logger(logger), _tickerCfg(_tickerCfg)
    {
        tradeEngine->_algoOnOrderBookUpdate = [this](auto tickerId, auto price, auto side, auto book) { onOrderBookUpdate(tickerId, price, side, book); };
        tradeEngine->_algoOnTradeUpdate     = [this](auto marketUpdate, auto book) { onTradeUpdate(marketUpdate, book); };
        tradeEngine->_algoOnOrderUpdate     = [this](auto clientResponse) { onOrderUpdate(clientResponse); };
    }

    void LiquidityTaker::onOrderBookUpdate(TickerId tickerId, Price price, Side side, const order_book::MarketOrderBook* book) noexcept
    {
        (void) book;
        _logger->log("%:% %() % ticker:% price:% side:%\n",
            __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), tickerId, priceToString(price).c_str(), sideToString(side).c_str());
    }
    void LiquidityTaker::onTradeUpdate(const MarketUpdate* marketUpdate, const order_book::MarketOrderBook* book) noexcept
    {
        _logger->log("%:% %() % %\n",
            __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), marketUpdate->toString().c_str());
        const auto bbo = book->getBBO();
        const auto aggQtyRatio = _featureEngine->getAggTradeQtyRatio();
    
        if (bbo->_bidPrice != PRICE_INVALID && bbo->_askPrice != PRICE_INVALID && aggQtyRatio!= FEATURE_INVALID) [[ likely ]]
        {
            _logger->log("%:% %() % % aggQtyRatio:%\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), bbo->toString().c_str(), aggQtyRatio);
            const auto clip         = _tickerCfg.at(marketUpdate->_tickerId)._clip;
            const auto threshold    = _tickerCfg.at(marketUpdate->_tickerId)._threshold;

            if (aggQtyRatio >= threshold)
            {
                if (marketUpdate->_side == Side::BUY)
                {
                    _orderManager->moveOrders(marketUpdate->_tickerId, bbo->_askPrice, PRICE_INVALID, clip);
                }
                else
                {
                    _orderManager->moveOrders(marketUpdate->_tickerId, PRICE_INVALID, bbo->_bidPrice, clip);
                }
            }
        }
    }
    void LiquidityTaker::onOrderUpdate(const ClientResponse* clientResponse) noexcept
    {
        _logger->log("%:% %() % %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), clientResponse->toString().c_str());
        _orderManager->onOrderUpdate(clientResponse);
    }
}
