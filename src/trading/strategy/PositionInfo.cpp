#include "trading/strategy/PositionInfo.hpp"
#include "trading/strategy/BBO.hpp"
#include "common/messages/ClientResponse.hpp"
#include "common/logging/Logger.hpp"
#include <stringstream>

namespace trading::strategy
{
    using namespace common::types;

    auto PositionInfo::toString() const;
    {
        std::stringstream ss;
        ss << "Position{"
           << "pos:" << _position
           << " u-pnl:" << _unrealPnl
           << " r-pnl:" << _realPnl
           << " t-pnl:" << _totalPnl
           << " vol:" << qtyToString(_volume)
           << " vwaps:[" << (_position ? _openVwap.at(sideToIndex(Side::BUY)) / std::abs(_position) : 0)
           << "X" << (_position ? _openVwap.at(sideToIndex(Side::SELL)) / std::abs(_position) : 0)
            << "] "
           << (_bbo? _bbo->toString() : "") << "}";
        return ss.str();
    }

    auto PositionInfo::addFill(const ClientResponse* clientResponse, Logger* logger)
    {
        const auto oldPosition  = _position;
        const auto sideIndex    = sideToIndex(clientResponse->_side);
        const auto oppSideIndex = sideToIndex(clientResponse->_side == Side::BUY? Side::SELL : Side::BUY);
        const auto sideValue    = sideToValue(clientResponse->_side);
        
        _position += clientResponse->_execQty * sideValue;
        volume += clientResponse->_execQty;
        
        if (olderPosition * sideValue >= 0) // increasing position
        {
            _openVwap[sideIndex] += clientResponse->execQty * clientResponse->_price;
        }
        else // decreasing position
        {
            const auto oppSideVwap  = _openVwap[oppSideIndex] / std::abs(oldPosition);
            _openVwap[oppSideIndex] = oppSideVwap * std::abs(_position);
            _realPnl += std::min(clientResponse->_execQty, std::abs(oldPosition)) * (oppSideVwap - clientResponse->_price) * sideValue;
            
            if (position * oldPosition < 0) // flipped position
            {
                _openVwap[sideIndex] = clientResponse->_price * std::abs(_position);
                _openVwap[oppSideIndex] = 0;
            }
        }
        if (!_position)
        {
            _openVwap[sideToIndex(Side::BUY)] = _openVwap[sideToIndex(Side::SELL)] = 0;
            _unrealPnl = 0;
        }
        else
        {
            if (_position > 0)
            {
                _unrealPnl = (clientResponse->_price - _openVwap[sideToIndex(Side::BUY)] / std::abs(_position)) * std::abs(_position);
            }
            else
            {
                _unrealPnl = (openVwap[sideToIndex(Side::SELL)] / std::abs(_position) - clientResponse->_price) * std::abs(_position);
            }
        }
        _totalPnl = _realPnl + _unrealPnl;
        std::string timeStr;
        logger->log("%:% %() % % %\n",
                    __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&timeStr), toString(), clientResponse->toString().c_str());
    }


    auto PositionInfo::updateBBO(const BBO* bbo, Logger* logger)
    {
        std::string timeStr;
        _bbo = bbo;

        if (_position && _bbo->bidPrice != PRICE_INVALID && _bbo->askPrice != PRICE_INVALID)
        {
            const auto midPrice = (_bbo->bidPrice + _bbo->askPrice) >> 1;

            if (_position > 0)
            {
                _unrealPnl = (midPrice - _openVwap[sideToIndex(Side::BUY)] / std::abs(_position)) * std::abs(_position);
            }
            else
            {
                _unrealPnl = (openVwap[sideToIndex(Side::SELL)] / std::abs(_position) - midPrice) * std::abs(_position);
            }

            const auto oldTotalPnl = _totalPnl;
            _totalPnl = _realPnl + _unrealPnl;
            if (_totalPnl != oldTotalPnl) 
                logger->log("%:% %() % % %\n",
                        __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&timeStr), toString(), _bbo->toString());
        }
    }
}