#include "InpStrm.hpp"

std::vector<std::string> splitToVector(const std::string &str)
{
  // Split string on delimiter (',') and remove leading/trailing spaces from each element
  std::stringstream ss(str);
  std::vector<std::string> elements;
  std::string element;
  while (std::getline(ss, element, ','))
  {
    // Trim leading and trailing spaces from the element
    element.erase(0, element.find_first_not_of(" "));
    element.erase(element.find_last_not_of(" ") + 1);
    elements.push_back(element);
  }
  return elements;
}

bool sendMessage(udp::socket &socket, const udp::endpoint &receiver_endpoint, const std::string &base_msg)
{
  try
  {
    socket.send_to(boost::asio::buffer(base_msg), receiver_endpoint);
    return true;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error sending message: " << e.what() << std::endl;
    return false;
  }
}

// Function to parse a single "N" (AddOrderMsg) message
bool parseAddOrderMsg(const std::string &str, msgs::AddOrderMsg &addOrderMsg)
{
  std::vector<std::string> elements = splitToVector(str);
  if (elements.size() == 7 && elements[0] == "N")
  {
    if (elements[5].size() != 1 || (elements[5][0] != 'S' && elements[5][0] != 'B'))
      return false;

    try
    {
      addOrderMsg.set_userid(std::stoi(elements[1]));
      addOrderMsg.set_symbol(elements[2]);
      addOrderMsg.set_price(std::stoi(elements[3]));
      addOrderMsg.set_quantity(std::stoi(elements[4]));
      addOrderMsg.set_isbuy(elements[5][0] == 'B');
      addOrderMsg.set_orderid(std::stoi(elements[6]));
      return true;
    }
    catch (const std::exception &e)
    {
      return false;
    }
  }
  return false;
}

// Function to parse a single "C" (CancelOrderMsg) message
bool parseCancelOrderMsg(const std::string &str, msgs::CancelOrderMsg &cancelOrderMsg)
{
  std::vector<std::string> elements = splitToVector(str);
  if (elements.size() == 3 && elements[0] == "C")
  {
    try
    {
      cancelOrderMsg.set_userid(std::stoi(elements[1]));
      cancelOrderMsg.set_orderid(std::stoi(elements[2]));
      return true;
    }
    catch (const std::exception &e)
    {
      return false;
    }
  }
  return false;
}

// Function to parse a single "F" (ClearBooksMsg) message
bool parseClearBooksMsg(const std::string &str, msgs::ClearBooksMsg &clearBooksMsg)
{
  std::vector<std::string> elements = splitToVector(str);
  if (elements.size() == 1 && elements[0] == "F")
  {
    return true;
  }
  return false;
}

std::string serBaseMessage(msgs::MessageType type, const std::string &payload)
{
  msgs::BaseMessage baseMessage;
  baseMessage.set_type(type);
  baseMessage.set_payload(payload);

  std::string serializedMsg;
  baseMessage.SerializeToString(&serializedMsg);

  return serializedMsg;
}
bool parseAndSend(const std::string &str, udp::socket &socket, const udp::endpoint &receiver_endpoint)
{
  // Dispatch to the appropriate message type handler
  msgs::AddOrderMsg addOrderMsg;
  if (parseAddOrderMsg(str, addOrderMsg))
  {
    std::string aos = serBaseMessage(msgs::MessageType::AddOrder, addOrderMsg.SerializeAsString());
    if (!sendMessage(socket, receiver_endpoint, aos))
      return false;
    return true;
  }
  msgs::CancelOrderMsg cancelOrderMsg;
  if (parseCancelOrderMsg(str, cancelOrderMsg))
  {
    std::string cos = serBaseMessage(msgs::MessageType::CancelOrder, cancelOrderMsg.SerializeAsString());
    if (!sendMessage(socket, receiver_endpoint, cos))
      return false;
    return true;
  }
  msgs::ClearBooksMsg clearBooksMsg;
  if (parseClearBooksMsg(str, clearBooksMsg))
  {
    std::string cbs = serBaseMessage(msgs::MessageType::ClearBooks, clearBooksMsg.SerializeAsString());
    if (!sendMessage(socket, receiver_endpoint, cbs))
      return false;
    return true;
  }
  else
    return false;
}

void sendFileOverUDP(const std::string &filename, const std::string &destination_address, int port)
{

  if (!std::filesystem::exists(filename))
  {
    std::cout << "File doesn' t exist" << std::endl;
    return;
  }

  boost::asio::io_service io_service;
  udp::resolver resolver(io_service);
  udp::resolver::query query(udp::v4(), destination_address, std::to_string(port));
  udp::endpoint receiver_endpoint = *resolver.resolve(query);
  udp::socket socket(io_service);
  // udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));

  std::ifstream file(filename);
  std::string line;

  socket.open(udp::v4());
  while (std::getline(file, line))
  {
    parseAndSend(line, socket, receiver_endpoint);
  }
  socket.close();
}
