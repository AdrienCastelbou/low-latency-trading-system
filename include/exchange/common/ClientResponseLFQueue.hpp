#pragma once
#include "../../common/data_structures/LFQueue.hpp"
#include "../../common/network/messages/ClientResponse.hpp"

using namespace common::network::messages;

namespace exchange::common
{
    using ClientResponseLFQueue = ::common::data_structures::LFQueue<ClientResponse>;
} // namespace exchange::common