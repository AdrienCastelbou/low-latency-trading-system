#include "common/messages/OMClientRequest.hpp"
#include <sstream>

using namespace ::common::types;

namespace common::messages
{
    std::string OMClientRequest::toString() const
    {
        std::stringstream ss;
        ss << "Public MarketUpdate"
           << " ["
           << " seq:" << _seqNum
           << " " << _clientRequest.toString()
           << "]";
        return ss.str();
    }
} // namespace common::messages