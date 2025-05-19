#include "trading/strategy/MarketMaker.hpp"
#include "common/logging/Logger.hpp"
#include "common/time/time.hpp"
#include "common/messages/MarketUpdate.hpp"
#include "trading/strategy/FeatureEngine.hpp"
#include "trading/strategy/TradeEngine.hpp"

namespace trading::strategy
{
    namespace om = order_management;
    namespace ob = order_book;
    using namespace common::logging;
    using namespace common::time;
    using namespace common::types;
    using namespace common::messages;

    MarketMaker::MarketMaker(Logger* logger, TradeEngine* tradeEngine, const FeatureEngine* featureEngine, om::OrderManager* orderManager, const TradeEngineCfgHashMap& _tickerCfg)
    : _featureEngine(featureEngine), _orderManager(orderManager), _logger(logger), _tickerCfg(_tickerCfg)
    {
        tradeEngine->_algoOnOrderBookUpdate = [this](auto tickerId, auto price, auto side, auto book) { onOrderBookUpdate(tickerId, price, side, book); };
        tradeEngine->_algoOnTradeUpdate     = [this](auto marketUpdate, auto book) { onTradeUpdate(marketUpdate, book); };
        tradeEngine->_algoOnOrderUpdate     = [this](auto clientResponse) { onOrderUpdate(clientResponse); };
    }

    void MarketMaker::onOrderBookUpdate(TickerId tickerId, Price price, Side side, const ob::MarketOrderBook* book) noexcept
    {
        _logger->log("%:% %() % ticker:% price:% side:%\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), tickerId, priceToString(price).c_str(), sideToString(side).c_str());
        const auto bbo = book->getBBO();
        const auto fairPrice = _featureEngine->getMktPrice();

        if (bbo->_bidPrice != PRICE_INVALID && bbo->_askPrice != PRICE_INVALID && fairPrice!= FEATURE_INVALID) [[ likely ]]
        {
            
            _logger->log("%:% %() % % fair price:%\n",
                        __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), bbo->toString().c_str(), fairPrice);
            const auto clip         = _tickerCfg.at(tickerId)._clip;
            const auto threshold    = _tickerCfg.at(tickerId)._threshold;
            const auto bidPrice     = bbo->_bidPrice - (fairPrice - bbo->_bidPrice >= threshold ? 0 : 1);
            const auto askPrice     = bbo->_askPrice + (bbo->_askPrice - fairPrice >= threshold? 0 : 1);

            _orderManager->moveOrders(tickerId, bidPrice, askPrice, clip);
        }
    }

    void MarketMaker::onTradeUpdate(const MarketUpdate* marketUpdate, const ob::MarketOrderBook* book) noexcept
    {
        (void)book;
        _logger->log("%:% %() % %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), marketUpdate->toString().c_str());
    }

    void MarketMaker::onOrderUpdate(const ClientResponse* clientResponse) noexcept
    {
        _logger->log("%:% %() % %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&_timeStr), clientResponse->toString().c_str());
        _orderManager->onOrderUpdate(clientResponse);
    }

}