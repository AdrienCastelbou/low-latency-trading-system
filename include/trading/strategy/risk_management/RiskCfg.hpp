#pragma once
#include "../../../common/types/types.hpp"
#include <sstream>

namespace trading::strategy::risk_management
{
    using namespace common::types;

    struct RiskCfg
    {
        Qty _maxOrderSize   = 0;
        Qty _maxPosition    = 0;
        double _maxLoss     = 0;

        auto toString() const -> std::string
        {
            std::stringstream ss;

            ss  << "RiskCfg{"
                << " max order size: " << qtyToString(_maxOrderSize)
                << " max position: " << qtyToString(_maxPosition)
                << " max loss: " << _maxLoss
                << "}";
            return ss.str();
        }
    }
}