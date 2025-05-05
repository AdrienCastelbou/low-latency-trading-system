#include "common/messages/MDPMarketUpdate.hpp"
#include <sstream>

using namespace ::common::types;

namespace common::messages
{
    std::string MDPMarketUpdate::toString() const
    {
        std::stringstream ss;
        ss << "Public MarketUpdate"
           << " ["
           << " seq:" << _seqNum
           << " " << _marketUpdate.toString()
           << "]";
        return ss.str();
    }
} // namespace common::messages