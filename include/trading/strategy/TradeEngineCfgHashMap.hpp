#pragma once
#include "../../../common/types/types.hpp"

namespace trading::strategy
{
    using namespace common::types;
    using TradeEngineCfgHashMap = std::array<TradeEngineCfg, MAX_TICKERS>;
}