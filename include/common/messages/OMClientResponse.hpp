#pragma once

#include "ClientResponse.hpp"

namespace common::messages
{
    #pragma pack(push, 1)
    struct OMClientResponse {
        size_t _seqNum = 0;
        ClientResponse _clientResponse;
        std::string toString() const;
      };
    #pragma pack(pop)
} // namespace common::messages