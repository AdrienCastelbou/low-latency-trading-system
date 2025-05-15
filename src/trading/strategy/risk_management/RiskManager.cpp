#include "trading/strategy/risk_management/RiskManager.hpp"
#include "common/logging/Logger.hpp"
#include "trading/strategy/PositionKeeper.hpp"
#include "trading/strategy/TradeEngineCfg.hpp"
#include "common/constants/constants.hpp"

namespace trading::strategy::risk_management
{
    RiskManager(logging::Logger* logger, const PositionKeeper* positionKeeper, const TradeEngineCfg& tickerCfg)
    : _logger(logger)
    {
        using namespace common::constants;
        using namespace common::types;

        for (TickerId i = 0; i < MAX_TICKERS; i++)
        {
            _tickerRisk.at(i)._positionInfo = positionKeeper->getPositionInfo(i);
            _tickerRisk.at(i)._riskCfg = tickerCfg.at[i]._riskCfg;
        }
    }

    RiskCheckResult RiskManager::checkPreTradeRisk(TickerId tickerId, Side side, Qty qty) const noexcept
    {
        return _tickerRisk.at(tickerId).checkPreTradeRisk(side, qty);
    }

}