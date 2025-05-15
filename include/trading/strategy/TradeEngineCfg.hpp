#pragma once
#include "../../common/types/types.hpp"
#include "risk_management/RiskCfg.hpp"
#include <sstream>

namespace trading::strategy
{
    using namespace common::types;

    struct TradeEngineCfg
    {
        Qty _clip = 0;
        double _threshold = 0;
        risk_management::RiskCfg _riskCfg;

        auto toString() const {
            std::stringstream ss;
            ss << "TradeEngineCfg{"
               << "clip:" << qtyToString(_clip) << " "
               << "thresh:" << _threshold << " "
               << "risk:" << _riskCfg.toString()
               << "}";
            return ss.str();
          }
    }
}