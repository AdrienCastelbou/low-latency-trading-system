#pragma once
#include <string>
#include <array>
#include "../../common/constants/constants.hpp"
#include "PositionInfo.hpp"

namespace common
{
    namespace messages
    {
        struct ClientResponse;
    }

    namespace logging
    {
        class Logger;
    }
}

namespace trading::strategy
{
    using namespace common::logging;
    using namespace common::constants;
    using namespace common::messages;

    struct BBO;
    
    class PositionKeeper
    {
        public:
            PositionKeeper(Logger* logger);

            void addFill(const ClientResponse* clientResponse) noexcept;
            void updateBBO(TickerId tickerId, const BBO* bbo) noexcept;
            auto getPositionInfo(TickerId tickerId) const noexcept;
            std::string toString() const;
        private:
            std::string _timeStr;
            Logger* _logger = nullptr;
            std::array<PositionInfo, MAX_TICKERS> _tickerPosition;

    };
}