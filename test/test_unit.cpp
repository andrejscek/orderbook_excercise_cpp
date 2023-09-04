// Must include the gtest header to use the testing library
#include <gtest/gtest.h>
#include <iostream>
#include "ExgBackE.hpp"
#include "InpStrm.hpp"

// Test the Order constructor
TEST(Unit_OrderTest, ConstructorTest)
{
    // Create an Order struct
    msgs::AddOrderMsg addOrderMsg;
    int seqNum = 1;
    // Check if the Order is valid
    EXPECT_TRUE(parseAddOrderMsg("N, 12, IBM, 10, 100, B, 31", addOrderMsg));

    // Check the properties of the Order struct
    Order order = Order(addOrderMsg);
    EXPECT_EQ(order.userId, 12);
    EXPECT_EQ(order.symbol, "IBM");
    EXPECT_EQ(order.price, 10);
    EXPECT_EQ(order.quantity, 100);
    EXPECT_EQ(order.side, 'B');
    EXPECT_EQ(order.orderId, 31);
}
TEST(Unit_OrderTest, ConstructorTestInvalid)
{
    // Create an Order struct with an invalid input string
    msgs::AddOrderMsg addOrderMsg;
    int seqNum = 1;
    // Check if the Order is not valid
    EXPECT_FALSE(parseAddOrderMsg("N, 12, IBM, , , B, 31", addOrderMsg));
}

// Test the OrderBook addOrder method
TEST(Unit_OrderBookTest, AddOrderTest)
{
    int seqNum = 1;
    msgs::AddOrderMsg buyOrderMsg;
    msgs::AddOrderMsg sellOrderMsg;

    OrderBook orderBook;
    parseAddOrderMsg("N, 12, IBM, 10, 100, B, 31", buyOrderMsg);
    parseAddOrderMsg("N, 132, IBM, 12, 90, S, 321", sellOrderMsg);

    // Add buy order and check if it's at the top of the book
    ASSERT_TRUE(orderBook.addOrder(buyOrderMsg));
    EXPECT_EQ(orderBook.getTopOfBook('B'), "B, 10, 100");

    // Add sell order and check if it's at the top of the book
    ASSERT_TRUE(orderBook.addOrder(sellOrderMsg));
    EXPECT_EQ(orderBook.getTopOfBook('S'), "S, 12, 90");
}

// Test the OrderBook cancelOrder method
TEST(Unit_OrderBookTest, CancelOrderTest)
{
    int seqNum = 1;
    msgs::AddOrderMsg buyOrderMsg;
    msgs::AddOrderMsg sellOrderMsg;

    OrderBook orderBook;
    parseAddOrderMsg("N, 12, IBM, 10, 100, B, 31", buyOrderMsg);
    parseAddOrderMsg("N, 132, IBM, 12, 90, S, 321", sellOrderMsg);

    // Add orders
    orderBook.addOrder(buyOrderMsg);
    orderBook.addOrder(sellOrderMsg);

    // Cancel buy order and check if it's canceled
    CancelBookResponse response = orderBook.cancelOrder(12, 31);
    EXPECT_TRUE(response.success);

    // Try to cancel a non-existent order and check if it's not canceled
    response = orderBook.cancelOrder(12345, 3);
    EXPECT_FALSE(response.success);
}

// Test the OrderBook matchOrder method
TEST(Unit_OrderBookTest, MatchOrderTest)
{
    int seqNum = 1;
    msgs::AddOrderMsg buyOrderMsg;
    msgs::AddOrderMsg sellOrderMsg;
    msgs::AddOrderMsg buyToMatchMsg;

    OrderBook orderBook;
    parseAddOrderMsg("N, 12, IBM, 10, 100, B, 31", buyOrderMsg);
    parseAddOrderMsg("N, 132, IBM, 12, 90, S, 321", sellOrderMsg);
    parseAddOrderMsg("N, 122, IBM, 10, 25, S, 331", buyToMatchMsg);

    // Add orders
    orderBook.addOrder(buyOrderMsg);
    orderBook.addOrder(sellOrderMsg);

    // Match the orders
    Order order(buyToMatchMsg);
    TradeNotification trade = orderBook.matchOrder(order);
    EXPECT_TRUE(trade.valid);
    EXPECT_EQ(trade.quantity, 25);
}

TEST(Unit_OrderBookTest, TopOfBookChanges)
{
    OrderBook orderBook;

    // Create and add buy and sell orders
    msgs::AddOrderMsg buyOrderMsg;
    msgs::AddOrderMsg sellOrderMsg;

    int seqNum = 1;
    parseAddOrderMsg("N, 12, IBM, 9, 100, B, 31", buyOrderMsg);
    parseAddOrderMsg("N, 132, IBM, 12, 100, S, 321", sellOrderMsg);

    orderBook.addOrder(buyOrderMsg);
    orderBook.addOrder(sellOrderMsg);

    // Check the top of the book for both sides
    EXPECT_EQ(orderBook.getTopOfBook('B'), "B, 9, 100");
    EXPECT_EQ(orderBook.getTopOfBook('S'), "S, 12, 100");

    // Add more orders and check if they are at the top of the book
    msgs::AddOrderMsg anotherBuyOrderMsg;
    msgs::AddOrderMsg anotherSellOrderMsg;

    parseAddOrderMsg("N, 22, IBM, 10, 75, B, 32", anotherBuyOrderMsg);
    parseAddOrderMsg("N, 232, IBM, 11, 50, S, 322", anotherSellOrderMsg);

    orderBook.addOrder(anotherBuyOrderMsg);
    orderBook.addOrder(anotherSellOrderMsg);

    EXPECT_EQ(orderBook.getTopOfBook('B'), "B, 10, 75");
    EXPECT_EQ(orderBook.getTopOfBook('S'), "S, 11, 50");
}

TEST(Unit_OrderBooks, TopOfMultipleBooks)
{
    boost::asio::io_context ioContext;
    ExchangeBackend backend(ioContext, "127.0.0.1", 1234);

    // Create and add orders to different books
    msgs::AddOrderMsg ibmBuyOrderMsg;
    msgs::AddOrderMsg ibmSellOrderMsg;
    msgs::AddOrderMsg applBuyOrderMsg;
    msgs::AddOrderMsg applSellOrderMsg;

    parseAddOrderMsg("N, 12, IBM, 50, 100, B, 31", ibmBuyOrderMsg);
    parseAddOrderMsg("N, 12, IBM, 55, 100, S, 31", ibmSellOrderMsg);
    parseAddOrderMsg("N, 12, APPL, 10, 100, B, 31", applBuyOrderMsg);
    parseAddOrderMsg("N, 12, APPL, 15, 100, S, 31", applSellOrderMsg);

    backend.books["IBM"].addOrder(ibmBuyOrderMsg);
    backend.books["IBM"].addOrder(ibmSellOrderMsg);
    backend.books["APPL"].addOrder(applBuyOrderMsg);
    backend.books["APPL"].addOrder(applSellOrderMsg);

    EXPECT_EQ(backend.books["IBM"].getTopOfBook('B'), "B, 50, 100");
    EXPECT_EQ(backend.books["IBM"].getTopOfBook('S'), "S, 55, 100");
    EXPECT_EQ(backend.books["APPL"].getTopOfBook('B'), "B, 10, 100");
    EXPECT_EQ(backend.books["APPL"].getTopOfBook('S'), "S, 15, 100");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}