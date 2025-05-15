#pragma once
#include <string>
#include "TickerRiskInfoHashMap.hpp"
#include "../../common/types/types.hpp"
#include "RiskCheckResult.hpp"

namespace common::logging
{
    class Logger;
}

namespace trading::strategy
{
    struct PositionKeeper;
    struct TradeEngineCfg;
}

namespace trading::strategy::risk_management
{
    using namespace common;
    using namespace trading::strategy;
    using namespace common::types;

    class RiskManager
    {
        public:
            RiskManager(logging::Logger* logger, const PositionKeeper* positionKeeper, const TradeEngineCfg& cfg);
            
            RiskCheckResult checkPreTradeRisk(TickerId tickerId, Side side, Qty qty) const noexcept;
        private:
            std::string _timeStr;
            logging::Logger* _logger = nullptr;
            std::string _timeStr;
            TickerRiskInfoHashMap _tickerRisk;
    }
    

}