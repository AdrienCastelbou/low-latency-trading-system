#pragma once
#include <array>
#include "../../common/types/types.hpp"
#include <string>

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
    using namespace common::logging;

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

        std::string toString() const;
        void addFill(const ClientResponse* clientResponse, Logger* logger);
        void updateBBO(const BBO* bbo, Logger* logger) noexcept;
    };
}