#pragma once

#ifndef O_B_HPP
#define O_B_HPP

#include <vector>
#include <string>
#include <chrono>
#include <msgs.pb.h>

struct TradeNotification
{
    int userIdBuy;
    int userOrderIdBuy;
    int userIdSell;
    int userOrderIdSell;
    int quantity;
    int price;
    bool valid = false;
    bool isnewTop = false;
};

struct Order
{
    std::string symbol;
    int price;
    int quantity;
    char side; // 'B' or 'S'
    int userId;
    int orderId;
    std::chrono::time_point<std::chrono::system_clock> timestamp = std::chrono::system_clock::now();
    bool isMarketOrder = false;
    Order(const msgs::AddOrderMsg &msg);
};

struct CancelBookResponse
{
    bool success;
    bool isTop;
    char side;
};

class OrderBook
{
public:
    bool addOrder(const Order &order);
    CancelBookResponse cancelOrder(const int &userId, const int &orderId);
    std::string getTopOfBook(char side);
    TradeNotification matchOrder(Order &order);

    std::vector<Order> bids;
    std::vector<Order> asks;

private:
    static bool compareBids(const Order &a, const Order &b);
    static bool compareAsks(const Order &a, const Order &b);

    template <typename Compare>
    void insertSorted(std::vector<Order> &orders, const Order &order, Compare comp);

    std::pair<bool, bool> cancelOrderInVector(std::vector<Order> &orders, const int &userId, const int &orderId);
};

#endif