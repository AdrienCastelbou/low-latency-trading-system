#pragma once
#include <cstdint>
#include <string>

namespace common::enums
{
    enum class ClientResponseType : uint8_t {
        INVALID = 0,
        ACCEPTED = 1,
        CANCELED = 2,
        FILLED = 3,
        CANCEL_REJECTED = 4
      };

    inline std::string clientResponseTypeToString(ClientResponseType type)
    {
        switch (type) {
            case ClientResponseType::ACCEPTED:
            return "ACCEPTED";
            case ClientResponseType::CANCELED:
            return "CANCELED";
            case ClientResponseType::FILLED:
                return "FILLED";
            case ClientResponseType::CANCEL_REJECTED:
                return "CANCEL_REJECTED";
            case ClientResponseType::INVALID:
                return "INVALID";
        }
    return "UNKNOWN";
    }
} // namespace exchange::common::enums
