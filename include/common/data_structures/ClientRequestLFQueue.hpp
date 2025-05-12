#pragma once
#include "LFQueue.hpp"
#include "../messages/ClientRequest.hpp"

namespace common::data_structures
{
    using ClientRequestLFQueue = LFQueue<messages::ClientRequest>;
} // namespace common::data_structures