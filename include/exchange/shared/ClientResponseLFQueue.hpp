#pragma once
#include "../../common/data_structures/LFQueue.hpp"
#include "../../common/messages/ClientResponse.hpp"

using namespace common::messages;

namespace exchange::shared
{
    using ClientResponseLFQueue = common::data_structures::LFQueue<ClientResponse>;
} // namespace exchange::common