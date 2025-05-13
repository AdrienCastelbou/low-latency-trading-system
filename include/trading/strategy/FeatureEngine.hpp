#pragma once

#include <string>
#include "../../common/types/types.hpp"
#include "MarketOrderBook.hpp"

namespace common
{
    namespace messages
    {
        struct MarketUpdate;
    }
    namespace logging
    {
        class Logger;
    }
}

namespace trading::strategy
{
    using namespace common::logging;
    using namespace common::types;
    using namespace common::messages;

    constexpr auto FEATURE_INVALID = std::numeric_limits<double>::quiet_NaN();

    class FeatureEngine
    {
        public:
            FeatureEngine(Logger* logger);
            ~FeatureEngine() = default;

            void onOrderBookUpdate(TickerId tickerId, Price price, Side side, MarketOrderBook* book) noexcept;
            void onTradeUpdate(const MarketUpdate* marketUpdate, MarketOrderBook* book) noexcept;
            double getMktPrice() const noexcept;
            double getAggTradeQtyRatio() const noexcept;
        private:
            std::string _timeStr;
            Logger* _logger = nullptr;
            double _mktPrice = FEATURE_INVALID;
            double _aggTradeQtyRatio = FEATURE_INVALID;
    };
}