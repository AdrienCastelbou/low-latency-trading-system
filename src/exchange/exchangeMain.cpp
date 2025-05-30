#include <csignal>
#include <unistd.h>
#include "common/common.hpp"
#include "common/time/time.hpp"
#include "common/data_structures/LFQueue.hpp"
#include "common/logging/Logger.hpp"
#include "common/constants/constants.hpp"
#include "exchange/matching_engine/MatchingEngine.hpp"
#include "exchange/market_data/MarketDataPublisher.hpp"
#include "exchange/order_server/OrderServer.hpp"

using namespace common;
using namespace common::time;
using namespace common::logging;
using namespace exchange;

Logger* logger = nullptr;
matching_engine::MatchingEngine* matchingEngine = nullptr;
market_data::MarketDataPublisher* marketDataPublisher = nullptr;
order_server::OrderServer* orderServer = nullptr;

void signalHandler(int signum)
{
    (void) signum;
    using namespace std::literals::chrono_literals;
    
    std::this_thread::sleep_for(10s);
    delete logger;
    delete matchingEngine;
    delete marketDataPublisher;
    delete orderServer;
    logger              = nullptr;
    matchingEngine      = nullptr;
    marketDataPublisher = nullptr;
    orderServer         = nullptr;
    std::this_thread::sleep_for(10s);
    std::exit(EXIT_SUCCESS);
}
int main(int, char**)
{
    logger = new common::logging::Logger("exchange_main.log");
    signal(SIGINT, signalHandler);
    const int sleepTime = 100 * 1000;
    common::data_structures::ClientRequestLFQueue clientRequests(common::constants::MAX_CLIENT_UPDATES);
    common::data_structures::ClientResponseLFQueue clientResponses(common::constants::MAX_CLIENT_UPDATES);
    common::data_structures::MarketUpdateLFQueue marketUpdates(common::constants::MAX_MARKET_UPDATES);
    std::string timeStr;
    logger->log("%:% %() % Starting Matching Engine...\n",
                __FILE__, __LINE__, __FUNCTION__,
                common::time::getCurrentTimeStr(&timeStr));
    
    matchingEngine = new matching_engine::MatchingEngine(&clientRequests, &clientResponses, &marketUpdates);
    matchingEngine->start();
    const std::string mktPubIface = "lo";
    const std::string snapPubIp = "233.252.14.1", incPubIp = "233.252.14.3";
    const int snapPubPort = 20000, incPubPort = 20001;
    logger->log("%:% %() % Starting Market Data Publisher...\n",
                __FILE__, __LINE__, __FUNCTION__,
                common::time::getCurrentTimeStr(&timeStr));
    marketDataPublisher = new market_data::MarketDataPublisher(&marketUpdates, mktPubIface, snapPubIp, snapPubPort, incPubIp, incPubPort);
    marketDataPublisher->start();
    const std::string orderGwIface = "lo";
    const int orderGwPort = 12345;
    logger->log("%:% %() % Starting Order Server...\n",
                __FILE__, __LINE__, __FUNCTION__,
                common::time::getCurrentTimeStr(&timeStr));
    orderServer = new order_server::OrderServer(&clientRequests, &clientResponses, orderGwIface, orderGwPort);
    orderServer->start();
    while (true)
    {
        logger->log("%:% %() % Sleeping for a few milliseconds..\n", __FILE__, __LINE__, __FUNCTION__,
                    common::time::getCurrentTimeStr(&timeStr));
        usleep(sleepTime * 1000);
    }
}
