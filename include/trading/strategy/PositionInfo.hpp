#pragma once
#include <array>
#include "../../common/types/types.hpp"
#include <stringstream>

namespace common
{
    namespace logging
    {
        class Logger;
    }
    namespace messages
    {
        struct ClientResponse;
    }
}

namespace trading::strategy
{
    using namespace common::types;
    using namespace common::messages;

    struct BBO;

    struct PositionInfo
    {
        int32_t _position   = 0;
        double _realPnl     = 0;
        double _unrealPnl   = 0;
        double _totalPnl    = 0;
        std::array<double, sideToIndex(Side::MAX) + 1> _openVwap;
        Qty _volume         = 0;
        const BBO* _bbo     = nullptr;

        auto toString() const;
        auto addFill(const ClientResponse* clientResponse, Logger* logger);
        auto updateBBO(const BBO* bbo, Logger* logger) noexcept;
    };
}