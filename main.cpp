#include "include/common/common.hpp"
#include "include/common/time/time.hpp"
#include "include/common/data_structures/LFQueue.hpp"
#include "include/common/logging/Logger.hpp"
#include "include/common/network/network.hpp"
#include "include/common/network/TCPSocket.hpp"
#include "include/common/network/TCPServer.hpp"
#include "include/common/types/types.hpp"
#include "include/exchange/matching_engine/MatchingEngine.hpp"
#include <csignal>
#include "include/exchange/shared/constants/constants.hpp"
#include <unistd.h>
using namespace common;
using namespace common::time;
using namespace common::network;
using namespace common::logging;

Logger* logger = nullptr;
exchange::matching_engine::MatchingEngine* matchingEngine = nullptr;

void sig_handler(int)
{
  using namespace std::literals::chrono_literals;
  std::this_thread::sleep_for(10s);
  delete logger; logger = nullptr;
  delete matchingEngine; matchingEngine = nullptr;
  std::this_thread::sleep_for(10s);
  exit(EXIT_SUCCESS);
}

int main(int, char **) {
  logger = new Logger("exchange_main.log");
  std::signal(SIGINT, sig_handler);
  const int sleep_time = 100 * 1000;
  exchange::shared::ClientRequestLFQueue client_requests(exchange::shared::constants::MAX_CLIENT_UPDATES);
  exchange::shared::ClientResponseLFQueue client_responses(exchange::shared::constants::MAX_CLIENT_UPDATES);
  exchange::shared::MarketUpdateLFQueue market_updates(exchange::shared::constants::MAX_CLIENT_UPDATES);

  std::string time_str;
  logger->log("%:% %() % Starting Matching Engine...\n",
    __FILE__, __LINE__, __FUNCTION__,
    common::time::getCurrentTimeStr(&time_str));
    matchingEngine = new
    exchange::matching_engine::MatchingEngine(&client_requests, &client_responses, &market_updates, *logger);
    matchingEngine->start();
  while (true) {
    logger->log("%:% %() % Sleeping for a few milliseconds..\n", __FILE__, __LINE__, __FUNCTION__,
    common::time::getCurrentTimeStr(&time_str));
    usleep(sleep_time * 1000);
  }

}