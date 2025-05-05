#pragma once
#include "../../common/data_structures/LFQueue.hpp"
#include "../../common/messages/MDPMarketUpdate.hpp"

using namespace common::messages;

namespace exchange::shared
{
    using MDPMarketUpdateLFQueue = common::data_structures::LFQueue<MDPMarketUpdate>;
} // namespace exchange::common