#include "trading/strategy/PositionKeeper.hpp"
#include <sstream>
#include "common/types/types.hpp"
#include "trading/strategy/BBO.hpp"
#include "common/messages/ClientResponse.hpp"

namespace trading::strategy
{    
    using namespace common::messages;

    PositionKeeper::PositionKeeper(Logger* logger) : _logger(logger)
    {}

    const PositionInfo* PositionKeeper::getPositionInfo(TickerId tickerId) const noexcept
    {
        return &(_tickerPosition.at(tickerId));
    }

    void PositionKeeper::addFill(const ClientResponse* clientResponse) noexcept
    {
        _tickerPosition.at(clientResponse->_tickerId).addFill(clientResponse, _logger);
    }

    void PositionKeeper::updateBBO(TickerId tickerId, const BBO* bbo) noexcept
    {
        _tickerPosition.at(tickerId).updateBBO(bbo, _logger);
    }

    std::string PositionKeeper::toString() const 
    {
        using namespace common::types;

        double totalPnL = 0.0;
        Qty totalVolume = 0;
        std::stringstream ss;

        for (TickerId i = 0; i < _tickerPosition.size(); ++i)
        {
            ss << "TickerId: " << tickerIdToString(i) << " " << _tickerPosition.at(i).toString() << "\n";
            totalPnL += _tickerPosition.at(i)._totalPnl;
            totalVolume += _tickerPosition.at(i)._volume; 
        }

        ss << "Total PnL: " << totalPnL << " " << "Total Volume: " << totalVolume << "\n";
        return ss.str();
    }
}