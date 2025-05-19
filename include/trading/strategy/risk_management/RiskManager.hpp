#pragma once
#include <string>
#include "TickerRiskInfoHashMap.hpp"
#include "../../../common/types/types.hpp"
#include "RiskCheckResult.hpp"
#include "../TradeEngineCfgHashMap.hpp"
namespace common::logging
{
    class Logger;
}

namespace trading::strategy
{
    class PositionKeeper;
}

namespace trading::strategy::risk_management
{
    using namespace common;
    using namespace trading::strategy;
    using namespace common::types;

    class RiskManager
    {
        public:
            RiskManager(logging::Logger* logger, const PositionKeeper* positionKeeper, const TradeEngineCfgHashMap& cfg);
            
            RiskCheckResult checkPreTradeRisk(TickerId tickerId, Side side, Qty qty) const noexcept;
        private:
            logging::Logger* _logger = nullptr;
            std::string _timeStr;
            TickerRiskInfoHashMap _tickerRisk;
    };
}