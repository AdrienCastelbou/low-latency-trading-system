#include "common/messages/OMClientResponse.hpp"
#include <sstream>

using namespace ::common::types;

namespace common::messages
{
    std::string OMClientResponse::toString() const
    {
        std::stringstream ss;
        ss << "Public MarketUpdate"
           << " ["
           << " seq:" << _seqNum
           << " " << _clientResponse.toString()
           << "]";
        return ss.str();
    }
} // namespace common::messages