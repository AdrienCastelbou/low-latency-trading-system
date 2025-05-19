#pragma once

namespace trading::strategy::risk_management
{
    enum class RiskCheckResult : int8_t
    {
        INVALID = 0,
        ORDER_TOO_LARGE = 1,
        POSITION_TOO_LARGE = 2,
        LOSS_TOO_LARGE = 3,
        ALLOWED = 4
    };

    inline auto riskCheckResultToString(RiskCheckResult riskCheckResult) -> std::string
    {
        switch (riskCheckResult)
        {
            using enum RiskCheckResult;
            case INVALID:
                return "INVALID";
            case ORDER_TOO_LARGE:
                return "ORDER_TOO_LARGE";
            case POSITION_TOO_LARGE:
                return "POSITION_TOO_LARGE";
            case LOSS_TOO_LARGE:
                return "LOSS_TOO_LARGE";
            case ALLOWED:
                return "ALLOWED";
        }
        return "";
    }
}