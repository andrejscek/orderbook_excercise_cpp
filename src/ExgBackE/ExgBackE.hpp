#pragma once

#ifndef EXCHANGE_BACKEND_HPP
#define EXCHANGE_BACKEND_HPP

#include "ob.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <deque>
#include <google/protobuf/any.pb.h>
#include <msgs.pb.h>

const int MAX_MSG_SIZE = 128;
using boost::asio::ip::udp;

class ExchangeBackend
{
public:
  ExchangeBackend(boost::asio::io_context &ioContext, std::string address, int port, bool print_output_msg = false);
  void start();
  void stop();
  std::vector<std::string> getPrintOutput();
  std::unordered_map<std::string, OrderBook> books;

private:
  void Receive();
  void handleExgMessages();

  msgs::BaseMessage PopAndDeserializeMsg();
  void processMessage();
  void addToPrintQue(const std::string &msg);
  void handleNewOrder(const msgs::AddOrderMsg &msg);
  void handleCancel(const msgs::CancelOrderMsg &msg);
  void flushOrderBooks();
  void publishTrade(const TradeNotification &tn);
  void publishTopOfBook(const std::string &symbol, char side);

  boost::asio::io_service io_service_;
  boost::asio::io_context &ioContext_;
  udp::socket socket_;
  udp::endpoint senderEndpoint_;
  std::array<char, MAX_MSG_SIZE> receiveBuffer_;
  std::deque<std::pair<std::array<char, MAX_MSG_SIZE>, int>> received_msg_deueue_;
  std::mutex received_msg_mutex_;
  std::mutex out_msg_mutex_;
  std::deque<std::string> out_msg_deque_;

  bool running_;
  bool print_output_msg;

  std::thread message_thread_;
  std::thread receive_thread_;
  std::thread user_thread_;
  std::thread processing_thread_;
  std::condition_variable msg_cv;

  std::vector<std::string> print_output_;
  std::mutex print_out_mutex_;
};

#endif