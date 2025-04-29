#pragma once
#include "../../common/data_structures/LFQueue.hpp"
#include "../../common/network/messages/MarketUpdate.hpp"

using namespace common::network::messages;

namespace exchange::common
{
    using ClientResponseLFQueue = ::common::data_structures::LFQueue<MarketUpdate>;
} // namespace exchange::common