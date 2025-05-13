#pragma once

#include <limits>
#include <cstdint>
#include <string>

namespace common::types
{
    using OrderId = uint64_t;
    constexpr auto ORDERID_INVALID = std::numeric_limits<OrderId>::max();

    inline std::string orderIdToString(OrderId orderId)
    {
        if (orderId == ORDERID_INVALID) [[ unlikely ]]
        {
            return "INVALID";
        }

        return std::to_string(orderId);
    }

    using TickerId = uint32_t;
    constexpr auto TICKERID_INVALID = std::numeric_limits<TickerId>::max();

    inline std::string tickerIdToString(TickerId tickerId)
    {
        if (tickerId == TICKERID_INVALID) [[ unlikely ]]
        {
            return "INVALID";
        }

        return std::to_string(tickerId);
    }

    using ClientId = uint32_t;
    constexpr auto CLIENTID_INVALID = std::numeric_limits<ClientId>::max();

    inline std::string clientIdToString(ClientId clientId)
    {
        if (clientId == CLIENTID_INVALID) [[ unlikely ]]
        {
            return "INVALID";
        }

        return std::to_string(clientId);
    }

    using Price = int64_t;
    constexpr auto PRICE_INVALID = std::numeric_limits<Price>::max();

    inline std::string priceToString(Price price)
    {
        if (price == PRICE_INVALID) [[ unlikely ]]
        {
            return "INVALID";
        }

        return std::to_string(price);
    }

    using Qty = uint32_t;
    constexpr auto QTY_INVALID = std::numeric_limits<Qty>::max();

    inline std::string qtyToString(Qty qty)
    {
        if (qty == QTY_INVALID) [[ unlikely ]]
        {
            return "INVALID";
        }

        return std::to_string(qty);
    }


    using Priority = uint64_t;
    constexpr auto PRIORITY_INVALID = std::numeric_limits<Priority>::max();

    inline std::string priorityToString(Priority priority)
    {
        if (priority == PRIORITY_INVALID) [[ unlikely ]]
        {
            return "INVALID";
        }

        return std::to_string(priority);
    }

    enum class Side : int8_t
    {
        INVALID = 0,
        BUY     = 1,
        SELL    = -1
    };

    inline std::string sideToString(Side side)
    {
        switch (side)
        {
            case Side::BUY:
                return "BUY";
            case Side::SELL:
                return "SELL";
            case Side::INVALID:
                return "INVALID";
            default:
                return "UNKNOW";
        }
    }

    inline constexpr auto sideToIndex(Side side) noexcept
    {
        return static_cast<int8_t>(side) + 1;
    }

    inline constexpr auto sideToValue(Side side) noexcept
    {
        return static_cast<int8_t>(side);
    }
    // TEST
    /*template<typename T, T tInvalid>
    inline std::string valueToString(T value)
    {
        if (value == tInvalid)
        {
            return "INVALID";
        }
        return std::to_string(value);
    }

    template std::string valueToString<OrderId,     ORDERID_INVALID>(OrderId);
    template std::string valueToString<TickerId,    TICKERID_INVALID>(TickerId);
    template std::string valueToString<ClientId,    CLIENTID_INVALID>(ClientId);
    template std::string valueToString<Price,       PRICE_INVALID>(Price);
    template std::string valueToString<Qty,         QTY_INVALID>(Qty);*/

} // namespace common::types
