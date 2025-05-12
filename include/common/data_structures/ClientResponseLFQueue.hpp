#pragma once
#include "LFQueue.hpp"
#include "../messages/ClientResponse.hpp"

namespace common::data_structures
{
    using ClientResponseLFQueue = LFQueue<messages::ClientResponse>;
} // namespace common::data_structures