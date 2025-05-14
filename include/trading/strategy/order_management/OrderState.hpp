#pragma once

namespace trading::strategy::order_management
{
    enum class OrderState : int8_t
    {
        INVALID = 0,
        PENDING_NEW = 1,
        LIVE = 2,
        PENDING_CANCEL = 3,
        DEAD = 4
    };

    inline auto OrderStateToString(OrderState state) -> std::string
    {
        switch (state)
        {
            using enum OrderState;
            case PENDING_NEW:
                return "PENDING_NEW";
            case LIVE:
                return "LIVE";
            case PENDING_CANCEL:
                return "PENDING_CANCEL";
            case DEAD:
                return "DEAD";
            case INVALID:
                return "INVALID";
            default:
                return "UNKNOWN";
        }
    }
}