#include "include/common/common.hpp"
#include "include/common/time/time.hpp"
#include "include/common/data_structures/LFQueue.hpp"
#include "include/common/logging/Logger.hpp"
#include "include/common/network/network.hpp"
#include "include/common/network/TCPSocket.hpp"
#include "include/common/network/TCPServer.hpp"

using namespace common;
using namespace common::time;
using namespace common::network;
using namespace common::logging;

int main(int, char **) {
  std::string time_str_;
  Logger logger_("socket_example.log");
  auto tcpServerRecvCallback = [&](TCPSocket *socket, Nanos
    rx_time) noexcept{
      logger_.log("TCPServer::defaultRecvCallback() socket:% len:% rx:%\n", socket->getFd(), socket->getNextRcvValidIndex(), rx_time);
      const std::string reply = "TCPServer received msg:" +
        std::string(socket->getRcvBuffer(), socket->getNextRcvValidIndex());
      socket->setNextRcvValidIndex(0);
      socket->send(reply.data(), reply.length());
  };
  auto tcpServerRecvFinishedCallback = [&]() noexcept{
    logger_.log("TCPServer::defaultRecvFinishedCallback()\n");
  };
  auto tcpClientRecvCallback = [&](TCPSocket *socket, Nanos rx_time) noexcept{
      const std::string recv_msg = std::string(socket->getRcvBuffer(), socket->getNextRcvValidIndex());
      socket->setNextRcvValidIndex(0);
      logger_.log("TCPSocket::defaultRecvCallback() socket:% len:% rx:% msg:%\n",
      socket->getFd(), socket->getNextRcvValidIndex(), rx_time,
        recv_msg);
  };

  const std::string iface = "lo";
  const std::string ip = "127.0.0.1";
  const int port = 12345;
  logger_.log("Creating TCPServer on iface:% port:%\n",
    iface, port);
  TCPServer server(logger_);
  server.setRecvCallback(tcpServerRecvCallback);
  server.setRecvFinishedCallback(tcpServerRecvFinishedCallback);

  server.listen(iface, port);
  std::vector < TCPSocket * > clients(5);
  for (size_t i = 0; i < clients.size(); ++i) {
    clients[i] = new TCPSocket(logger_);
    clients[i]->setRecvCallback(tcpClientRecvCallback);
    logger_.log("Connecting TCPClient-[%] on ip:% iface:% port:%\n", i, ip, iface, port);
    clients[i]->connect(ip, iface, port, false);
    server.poll();
  }

  using namespace std::literals::chrono_literals;
  for (auto itr = 0; itr < 5; ++itr) {
    for (size_t i = 0; i < clients.size(); ++i) {
      const std::string client_msg = "CLIENT-[" +
        std::to_string(i) + "] : Sending " +
          std::to_string(itr * 100 + i);
      logger_.log("Sending TCPClient-[%] %\n", i,
        client_msg);
      clients[i]->send(client_msg.data(),
        client_msg.length());
      clients[i]->sendAndRecv();
      std::this_thread::sleep_for(500ms);
      server.poll();
      server.sendAndRecv();
    }
  }
  for (auto itr = 0; itr < 5; ++itr) {
    for (auto &client: clients)
      client->sendAndRecv();
    server.poll();
    server.sendAndRecv();
    std::this_thread::sleep_for(500ms);
  }
  return 0;
}