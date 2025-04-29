#pragma once
#include <cstdint>
#include <string>

namespace common::enums
{
    enum class ClientRequestType : uint8_t
    {
        INVALID = 0,
        NEW     = 1,
        CANCEL  = 2
    };

    inline std::string clientRequestTypeToString(ClientRequestType type)
    {
        switch (type) {
        case ClientRequestType::NEW:
            return "NEW";
        case ClientRequestType::CANCEL:
            return "CANCEL";
        case ClientRequestType::INVALID:
            return "INVALID";
        }
        return "UNKNOWN";
    }
} // namespace exchange::common::enums
