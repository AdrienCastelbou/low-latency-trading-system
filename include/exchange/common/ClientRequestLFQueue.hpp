#pragma once
#include "../../common/data_structures/LFQueue.hpp"
#include "../../common/network/messages/ClientRequest.hpp"

using namespace common::network::messages;

namespace exchange::common
{
    using ClientRequestLFQueue = ::common::data_structures::LFQueue<ClientRequest>;
} // namespace exchange::common
