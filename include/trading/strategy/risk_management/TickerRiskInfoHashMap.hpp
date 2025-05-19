#pragma once

#include "RiskInfo.hpp"
#include "../../../common/constants/constants.hpp"

namespace trading::strategy::risk_management
{
    using namespace common::constants;

    using TickerRiskInfoHashMap = std::array<RiskInfo, MAX_TICKERS>;
}