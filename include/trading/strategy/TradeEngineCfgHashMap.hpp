#pragma once
#include "../../common/constants/constants.hpp"
#include "TradeEngineCfg.hpp"

namespace trading::strategy
{
    using namespace common::constants;
    using TradeEngineCfgHashMap = std::array<TradeEngineCfg, MAX_TICKERS>;
}