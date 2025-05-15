#pragma once
#include "RiskCfg.hpp"
#include "../../../common/types/types.hpp"
#include "RiskCheckResult.hpp"

namespace trading::strategy
{
    struct PositionInfo;

    namespace risk_management
    {
        using namespace common::types;

        struct RiskInfo
        {
            const PositionInfo* _positionInfo = nullptr;
            RiskCfg _riskCfg;

            std::string toString() const;
            RiskCheckResult checkPreTradeRisk(Side side, Qty qty) noexcept;
            ;
        };
    }
}