#pragma once

#include <functional>
#include <string>
#include "../../common/constants/constants.hpp"
#include "../../common/types/OrderTypes.hpp"

namespace trading::order_gateway
{
    using namespace common::types;

    class OrderGateway
    {
        private:
            const ClientId _clientId;
            std::string _ip;
            const std::string _iface;
            const int _port = 0;
            

    };
}