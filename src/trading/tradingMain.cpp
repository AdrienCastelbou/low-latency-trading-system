#include <csignal>
#include <unistd.h>
#include "trading/strategy/TradeEngine.hpp"
#include "trading/order_gateway/OrderGateway.hpp"
#include "trading/market_data/MarketDataConsumer.hpp"
#include "common/logging/Logger.hpp"
#include <vector>

common::logging::Logger* logger = nullptr;
trading::strategy::TradeEngine* tradeEngine = nullptr;
trading::order_gateway::OrderGateway* orderGateway = nullptr;
trading::market_data::MarketDataConsumer* marketDataConsumer = nullptr;

using namespace common;
using namespace trading;
using namespace trading::strategy::risk_management;
using namespace common::types;

int main(int argc, char** argv)
{
    types::ClientId clientId = atoi(argv[1]);
    srand(clientId);

    const auto algoType = strategy::stringToAlgoType(argv[2]);
    strategy::TradeEngineCfgHashMap tickerCfg;
    size_t nextTickerId = 0;

    for (int i = 3; i < argc; i+= 5, ++nextTickerId)
    {
        tickerCfg.at(nextTickerId) = {
            static_cast<types::Qty>(std::atoi(argv[i])),
            std::atof(argv[i+1]),
            {
                static_cast<types::Qty>(std::atoi(argv[i + 2])),
                static_cast<types::Qty>(std::atoi(argv[i + 3])),
                std::atof(argv[i + 4])
            }
        };
    }

    logger = new common::logging::Logger("trading_main_" + std::to_string(clientId) + ".log");
    const int sleepTime = 20 * 1000;
    data_structures::ClientRequestLFQueue clientRequests(constants::MAX_CLIENT_UPDATES);
    data_structures::ClientResponseLFQueue clientResponses(constants::MAX_CLIENT_UPDATES);
    data_structures::MarketUpdateLFQueue marketUpdates(constants::MAX_MARKET_UPDATES);
    
    std::string timeStr;
    logger->log("%:% %() % Starting trading engine...\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&timeStr));
    
    tradeEngine = new strategy::TradeEngine(clientId, algoType, tickerCfg, &clientRequests, &clientResponses, &marketUpdates);
    tradeEngine->start();

    const std::string order_gw_ip = "127.0.0.1";
    const std::string order_gw_iface = "lo";
    const int order_gw_port = 12345;
    logger->log("%:% %() % Starting order gateway...\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&timeStr));
    
    orderGateway = new trading::order_gateway::OrderGateway(clientId, &clientRequests, &clientResponses, order_gw_ip, order_gw_iface, order_gw_port);
    orderGateway->start();
    
    const std::string mkt_data_iface = "lo";
    const std::string snapshot_ip = "233.252.14.1";
    const int snapshot_port = 20000;
    const std::string incremental_ip = "233.252.14.3";
    const int incremental_port = 20001;
    logger->log("%:% %() % Starting market data consumer...\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&timeStr));

    marketDataConsumer = new trading::market_data::MarketDataConsumer(clientId, &marketUpdates, mkt_data_iface, snapshot_ip, snapshot_port, incremental_ip, incremental_port);
    marketDataConsumer->start();
    usleep(10 * 1000 * 1000);

    tradeEngine->initLastEventTime();
    if (algoType == AlgoType::RANDOM)
    {
        OrderId orderId = clientId * 1000;
        std::vector<messages::ClientRequest> clientRequestsVec;
        std::array<Price, constants::MAX_TICKERS> tickerBasePrices;
        
        for (size_t i = 0; i < constants::MAX_TICKERS; ++i)
        {
            tickerBasePrices[i] = rand() % 100 + 1000;
        }
        tradeEngine->initLastEventTime();

        for(size_t i = 0; i < 10000; ++i)
        {
            const TickerId tickerId = rand() % constants::MAX_TICKERS;
            const Price price = tickerBasePrices[tickerId] + (rand() % 10) + 1;
            const Qty qty = 1 + (rand() % 100) + 1;
            const Side side = rand() % 2 ? Side::BUY : Side::SELL;
            messages::ClientRequest newRequest{enums::ClientRequestType::NEW, clientId, tickerId, orderId++, side, price, qty};

            tradeEngine->sendClientRequest(&newRequest);
            usleep(sleepTime);
            clientRequestsVec.push_back(newRequest);
            
            const auto cancelIdx = rand() % clientRequestsVec.size();
            auto cancelRequest = clientRequestsVec[cancelIdx];
            cancelRequest._type = enums::ClientRequestType::CANCEL;
            tradeEngine->sendClientRequest(&cancelRequest);
            usleep(sleepTime);
        }
    }
    while (tradeEngine->silentSeconds() < 60)
    {
        logger->log("%:% %() % Waiting till no activity, been silent for % seconds...\n", __FILE__, __LINE__, __FUNCTION__, common::time::getCurrentTimeStr(&timeStr), tradeEngine->silentSeconds());
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(10s);
    }
    tradeEngine->stop();
    marketDataConsumer->stop();
    orderGateway->stop();
    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(10s);
    delete logger;
    logger = nullptr;
    delete tradeEngine;
    tradeEngine = nullptr;
    delete marketDataConsumer;
    marketDataConsumer = nullptr;
    delete orderGateway;
    orderGateway = nullptr;
    std::this_thread::sleep_for(10s);
    exit(EXIT_SUCCESS);
}