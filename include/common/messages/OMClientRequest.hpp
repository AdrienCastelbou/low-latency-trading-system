#pragma once

#include "ClientRequest.hpp"

namespace common::messages
{
    #pragma pack(push, 1)
    struct OMClientRequest {
        size_t _seqNum = 0;
        ClientRequest _clientRequest;
        std::string toString() const;
      };
    #pragma pack(pop)
} // namespace common::messages