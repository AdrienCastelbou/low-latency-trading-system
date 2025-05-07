#pragma once
#include "LFQueue.hpp"
#include "../messages/MarketUpdate.hpp"

using namespace common::messages;

namespace common::data_structures
{
    using MarketUpdateLFQueue = LFQueue<MarketUpdate>;
} // namespace exchange::common