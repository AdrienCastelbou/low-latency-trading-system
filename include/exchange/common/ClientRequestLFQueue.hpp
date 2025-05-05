#pragma once
#include "../../common/data_structures/LFQueue.hpp"
#include "../../common/messages/ClientRequest.hpp"

using namespace common::messages;

namespace exchange::common
{
    using ClientRequestLFQueue = ::common::data_structures::LFQueue<ClientRequest>;
} // namespace exchange::common
