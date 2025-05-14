#include "MarketOrderBook.hpp"
#include <array>
#include "../../../common/constants/constants.hpp"

namespace trading::strategy::order_book
{
    using namespace common::constants;
    using MarketOrderBookHashMap = std::array<MarketOrderBook*, MAX_TICKERS>;
}