#include "ExgBackE.hpp"

ExchangeBackend::ExchangeBackend(
    boost::asio::io_context &ioContext, std::string address, int port, bool print_output_msg) : ioContext_(ioContext),
                                                                                                socket_(ioContext, udp::endpoint(boost::asio::ip::make_address(address), port)),
                                                                                                running_(false),
                                                                                                print_output_msg(print_output_msg) {}

std::vector<std::string> ExchangeBackend::getPrintOutput()
{
  std::lock_guard<std::mutex> lock(print_out_mutex_);
  return print_output_;
}

void ExchangeBackend::start()
{
  running_ = true;

  Receive();
  receive_thread_ = std::thread([this]
                                { ioContext_.run(); });

  // Start the thread for processing exchange messages
  user_thread_ = std::thread(&ExchangeBackend::handleExgMessages, this);

  // Start a separate thread for processing received messages
  processing_thread_ = std::thread(&ExchangeBackend::processMessage, this);
}

void ExchangeBackend::stop()
{
  running_ = false;
  ioContext_.stop();

  receive_thread_.join();
  user_thread_.join();
  processing_thread_.join();
}

void ExchangeBackend::Receive()
{
  socket_.async_receive_from(
      boost::asio::buffer(receiveBuffer_), senderEndpoint_,
      [this](const boost::system::error_code &error, std::size_t bytesReceived)
      {
        if (!error && bytesReceived > 0)
        {
          std::lock_guard<std::mutex> lock(received_msg_mutex_);
          received_msg_deueue_.push_back(std::pair(receiveBuffer_, bytesReceived));
        }

        if (running_ && !error)
        {
          // Continue receiving
          Receive();
        }
      });
}
msgs::BaseMessage ExchangeBackend::PopAndDeserializeMsg()
{

  std::pair<std::array<char, MAX_MSG_SIZE>, int> buf_pair;
  {
    std::lock_guard<std::mutex> lock(received_msg_mutex_);
    buf_pair = received_msg_deueue_.front();
    received_msg_deueue_.pop_front();
  }
  msgs::BaseMessage baseMessage;
  baseMessage.ParseFromArray(buf_pair.first.data(), buf_pair.second);

  return baseMessage;
}

void ExchangeBackend::processMessage()
{
  while (running_)
  {
    if (received_msg_deueue_.empty())
    {
      std::this_thread::yield(); // Avoid CPU spin when the queue is empty
      continue;
    }

    msgs::BaseMessage msg = PopAndDeserializeMsg();

    if (msg.type() == msgs::MessageType::AddOrder)
    {
      msgs::AddOrderMsg addOrderMsg;
      addOrderMsg.ParseFromString(msg.payload());
      handleNewOrder(addOrderMsg);
    }
    else if (msg.type() == msgs::MessageType::CancelOrder)
    {
      msgs::CancelOrderMsg cancelOrderMsg;
      cancelOrderMsg.ParseFromString(msg.payload());
      handleCancel(cancelOrderMsg);
    }
    else if (msg.type() == msgs::MessageType::ClearBooks)
    {
      flushOrderBooks();
    }
    else
    {
      // Handle unknown message type
    }
  }
}

void ExchangeBackend::handleExgMessages()
{
  while (running_)
  {

    if (!out_msg_deque_.empty())
    {
      std::string msg;
      {
        std::lock_guard<std::mutex> lock(out_msg_mutex_);
        msg = out_msg_deque_.front();
        out_msg_deque_.pop_front();
      }
      {
        std::lock_guard<std::mutex> lock(print_out_mutex_);
        print_output_.push_back(msg); // for testing
      }
      if (print_output_msg)
        std::cout << msg << std::endl;
    }
  }
}

void ExchangeBackend::addToPrintQue(const std::string &msg)
{
  std::lock_guard<std::mutex> lock(out_msg_mutex_);
  out_msg_deque_.push_back(msg);
}

void ExchangeBackend::handleNewOrder(const msgs::AddOrderMsg &msg)
{
  Order order(msg);

  std::string ack = "A, " + std::to_string(order.userId) + ", " + std::to_string(order.orderId);
  addToPrintQue(ack);

  // match order and send notification untill no more matches
  bool hasTraded = false;
  while (true)
  {
    TradeNotification tn = books[order.symbol].matchOrder(order);
    if (!tn.valid)
      break;
    publishTrade(tn);
    hasTraded = true;
    // if (tn.isnewTop)  // better to publish after all matches are done, since the matches are done in 1 batch
    //   publishTopOfBook(order.symbol, order.side == 'B' ? 'S' : 'B');
    if (order.quantity == 0)
      break;
  }
  // publish book only after all matches are done and at least one trade occured
  if (hasTraded)
    publishTopOfBook(order.symbol, order.side == 'B' ? 'S' : 'B');

  // return if no order qty remains or is market order price = 0
  if (order.quantity == 0 || order.isMarketOrder)
    return;

  if (order.side == 'B')
  {
    bool isTop = books[order.symbol].addOrder(order);
    if (isTop)
      publishTopOfBook(order.symbol, order.side);
  }
  else if (order.side == 'S')
  {
    bool isTop = books[order.symbol].addOrder(order);
    if (isTop)
      publishTopOfBook(order.symbol, order.side);
  }
}

void ExchangeBackend::handleCancel(const msgs::CancelOrderMsg &msg)
{

  int oid = msg.orderid();
  int uid = msg.userid();
  for (auto &bookPair : books)
  {
    CancelBookResponse cancelR = bookPair.second.cancelOrder(uid, oid);
    if (cancelR.success)
    {
      std::string ack = "C, " + std::to_string(uid) + ", " + std::to_string(oid);
      addToPrintQue(ack);
      if (cancelR.isTop)
        publishTopOfBook(bookPair.first, cancelR.side);

      return;
    }
  }
}

void ExchangeBackend::flushOrderBooks()
{
  // does not publishTopOfBook according to output file
  // will emit empty string and end line
  for (auto &bookPair : books)
  {
    // Clear the bids and asks vectors for this book
    bookPair.second.bids.clear();
    bookPair.second.asks.clear();
  }
  addToPrintQue("");
};

void ExchangeBackend::publishTrade(const TradeNotification &tn)
{
  std::string ack = "T, " + std::to_string(tn.userIdBuy) + ", " + std::to_string(tn.userOrderIdBuy) + ", " + std::to_string(tn.userIdSell) + ", " + std::to_string(tn.userOrderIdSell) + ", " + std::to_string(tn.price) + ", " + std::to_string(tn.quantity);
  addToPrintQue(ack);
}
void ExchangeBackend::publishTopOfBook(const std::string &symbol, char side)
{
  // not in output file, no time to modify, but should be added, otherwise how do we now which book chaned?
  // std::string ack = "B, " + symbol + ", " "B, " + books[symbol].getTopOfBook(bs);
  std::string ack = "B, " + books[symbol].getTopOfBook(side);
  addToPrintQue(ack);
}
