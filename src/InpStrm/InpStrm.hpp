#pragma once

#ifndef INPUT_STREAMER_HPP
#define INPUT_STREAMER_HPP

#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <thread>
#include <filesystem>

#include "ob.hpp"
#include "msgs.pb.h"

using boost::asio::ip::udp;
std::vector<std::string> splitToVector(const std::string &str);
bool sendMessage(udp::socket &socket, const udp::endpoint &receiver_endpoint, const std::string &base_msg);
bool parseAddOrderMsg(const std::string &str, msgs::AddOrderMsg &addOrderMsg);
bool parseCancelOrderMsg(const std::string &str, msgs::CancelOrderMsg &cancelOrderMsg);
bool parseClearBooksMsg(const std::string &str, msgs::ClearBooksMsg &clearBooksMsg);
std::string serBaseMessage(msgs::MessageType type, const std::string &payload);
bool parseAndSend(const std::string &str, udp::socket &socket, const udp::endpoint &receiver_endpoint);
void sendFileOverUDP(const std::string &filename, const std::string &destination_address, int port);

#endif