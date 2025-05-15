#pragma once
#include <string>

namespace trading::strategy
{
    enum class AlgoType : int8_t
    {
        INVALID = 0,
        RANDOM  = 1,
        MAKER   = 2,
        TAKER   = 3,
        MAX     = 4
    };

    inline auto algoTypeToString(AlgoType algoType) noexcept -> std::string
    {
        switch (algoType)
        {
            using enum AlgoType;
            case INVALID: return "INVALID";
            case RANDOM:  return "RANDOM";
            case MAKER:   return "MAKER";
            case TAKER:   return "TAKER";
            case MAX:     return "MAX";
            default:      return "UNKNOWN"; // should never happen, but just in case, return UNKNOWN
        }
    }

    inline auto stringToAlgoType(const std::string& str) noexcept -> AlgoType
    {
        for (auto i = static_cast<int>(AlgoType::INVALID); i < static_cast<int>(AlgoType::MAX); i++)
        {
            const auto algoType = static_cast<AlgoType>(i);
            if (str == algoTypeToString(algoType))
            {
                return algoType;
            }
        }
        return AlgoType::INVALID;
    }
}