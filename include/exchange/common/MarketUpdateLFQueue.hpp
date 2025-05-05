#pragma once
#include "../../common/data_structures/LFQueue.hpp"
#include "../../common/messages/MarketUpdate.hpp"

using namespace common::messages;

namespace exchange::common
{
    using MarketUpdateLFQueue = ::common::data_structures::LFQueue<MarketUpdate>;
} // namespace exchange::common