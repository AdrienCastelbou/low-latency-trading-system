#include "trading/strategy/FeatureEngine.hpp"
#include "common/time/time.hpp"
#include "common/logging/Logger.hpp"
#include "common/messages/MarketUpdate.hpp"

namespace trading::strategy
{
    using namespace common::logging;
    using namespace common::time;
    using namespace common::messages;

    FeatureEngine::FeatureEngine(Logger* logger) : _logger(logger)
    {
    }

    void FeatureEngine::onOrderBookUpdate(TickerId tickerId, Price price, Side side, order_book::MarketOrderBook* book) noexcept
    {
        const auto bbo = book->getBBO();

        if (bbo->_bidPrice != PRICE_INVALID && bbo->_askPrice != PRICE_INVALID) [[ likely ]]
        {
            _mktPrice = (bbo->_bidPrice * bbo->_askQuantity + bbo->_askPrice * bbo->_bidQuantity) / static_cast<double>(bbo->_bidQuantity + bbo->_askQuantity);
        }

        _logger->log("%:% %() % ticker:% price:% side:% mkt-price:% agg-trade-ratio:%\n",
                    __FILE__, __LINE__, __FUNCTION__,
                    getCurrentTimeStr(&_timeStr), tickerId, priceToString(price).c_str(), sideToString(side).c_str(), _mktPrice, _aggTradeQtyRatio );
    }

    void FeatureEngine::onTradeUpdate(const MarketUpdate* marketUpdate, order_book::MarketOrderBook* book) noexcept
    {
        const auto bbo = book->getBBO();

        if (bbo->_bidPrice!= PRICE_INVALID && bbo->_askPrice!= PRICE_INVALID) [[ likely ]]
        {
            _aggTradeQtyRatio = static_cast<double>(marketUpdate->_qty) / (marketUpdate->_side == Side::BUY ? bbo->_askQuantity : bbo->_bidQuantity);
        }
        _logger->log("%:% %() % % mkt-price:% agg-trade-ratio:%\n",
                    __FILE__, __LINE__, __FUNCTION__,
                    getCurrentTimeStr(&_timeStr), marketUpdate->toString().c_str(), _mktPrice, _aggTradeQtyRatio );
    }

    double FeatureEngine::getMktPrice() const noexcept
    {
        return _mktPrice;
    }

    double FeatureEngine::getAggTradeQtyRatio() const noexcept
    {
        return _aggTradeQtyRatio;
    }
}