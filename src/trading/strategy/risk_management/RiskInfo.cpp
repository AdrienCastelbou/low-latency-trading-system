#include "trading/strategy/risk_management/RiskInfo.hpp"
#include "trading/strategy/PositionInfo.hpp"
#include <sstream>

namespace trading::strategy::risk_management
{
    std::string RiskInfo::toString() const
    {
        std::stringstream ss;

        ss  << "RiskInfo" << "["
            << "pos:" << _positionInfo->toString() << " "
            << _riskCfg.toString()
            << "]";
     return ss.str();
    }

    RiskCheckResult RiskInfo::checkPreTradeRisk(Side side, Qty qty) const noexcept
    {
        if (qty > _riskCfg._maxOrderSize) [[ unlikely ]]
        {
            return RiskCheckResult::ORDER_TOO_LARGE;
        }
        if (std::abs(_positionInfo->_position + sideToValue(side) * static_cast<int32_t>(qty)) > static_cast<int32_t>(_riskCfg._maxPosition)) [[ unlikely ]]
        {
            return RiskCheckResult::POSITION_TOO_LARGE;
        }
        if (_positionInfo->_totalPnl < _riskCfg._maxLoss) [[ unlikely ]]
        {
            return RiskCheckResult::LOSS_TOO_LARGE;
        }
        return RiskCheckResult::ALLOWED;
    }
}