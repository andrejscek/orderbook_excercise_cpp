#include "ob.hpp"
#include <sstream>
#include <algorithm>

Order::Order(const msgs::AddOrderMsg &msg)
{
    symbol = msg.symbol();
    price = msg.price();
    quantity = msg.quantity();
    side = msg.isbuy() ? 'B' : 'S';
    userId = msg.userid();
    orderId = msg.orderid();
    isMarketOrder = msg.price() == 0;
    timestamp = std::chrono::system_clock::now();
}

bool OrderBook::addOrder(const Order &order)
{
    if (order.side == 'B')
    {
        insertSorted(bids, order, compareBids);
        return (bids.size() > 0 && bids[0].orderId == order.orderId);
    }

    else if (order.side == 'S')
    {
        insertSorted(asks, order, compareAsks);
        return (asks.size() > 0 && asks[0].orderId == order.orderId);
    }

    return false; // unreachable
}

CancelBookResponse OrderBook::cancelOrder(const int &userId, const int &orderId)
{
    std::pair<bool, bool> cResB = cancelOrderInVector(bids, userId, orderId);
    std::pair<bool, bool> cResA = cancelOrderInVector(asks, userId, orderId);

    char side = (cResB.first) ? 'B' : 'S'; // assumes order can only be in either bids or asks

    return CancelBookResponse{cResB.first || cResA.first, cResB.second || cResA.second, side};
}

std::string OrderBook::getTopOfBook(char side)
{
    int topPrice = 0;
    int totalQuantity = 0;

    if (side == 'B' && !bids.empty())
    {
        topPrice = bids[0].price;
        for (const auto &bid : bids)
        {
            if (bid.price != topPrice)
                break;

            totalQuantity += bid.quantity;
        }
    }
    else if (side == 'S' && !asks.empty())
    {
        topPrice = asks[0].price;
        for (const auto &ask : asks)
        {
            if (ask.price != topPrice)
                break;

            totalQuantity += ask.quantity;
        }
    }

    std::string result = side + std::string(", ");
    result += (topPrice == 0) ? "-, -" : (std::to_string(topPrice) + ", " + std::to_string(totalQuantity));

    return result;
}

TradeNotification OrderBook::matchOrder(Order &order)
{
    if (order.side == 'B' && !asks.empty())
    {
        // check if the top of the book is a match
        if (order.price >= asks[0].price || order.isMarketOrder)
        {
            // match
            int tq = std::min(order.quantity, asks[0].quantity);
            TradeNotification tn;
            tn.userIdBuy = order.userId;
            tn.userOrderIdBuy = order.orderId;
            tn.userIdSell = asks[0].userId;
            tn.userOrderIdSell = asks[0].orderId;
            tn.quantity = tq;
            tn.price = asks[0].price;
            tn.valid = true;

            // fix structures set new top of book flag
            order.quantity -= tq;
            asks[0].quantity -= tq;
            tn.isnewTop = true; // publish even if only qty change
            if (asks[0].quantity == 0)
                asks.erase(asks.begin());

            return tn;
        }
    }
    else if (order.side == 'S' && !bids.empty())
    {
        // check if the top of the book is a match
        if (order.price <= bids[0].price || order.isMarketOrder)
        {
            // match
            int tq = std::min(order.quantity, bids[0].quantity);
            TradeNotification tn;
            tn.userIdBuy = bids[0].userId;
            tn.userOrderIdBuy = bids[0].orderId;
            tn.userIdSell = order.userId;
            tn.userOrderIdSell = order.orderId;
            tn.quantity = tq;
            tn.price = bids[0].price;
            tn.valid = true;

            // fix structures set new top of book flag
            order.quantity -= tq;
            bids[0].quantity -= tq;
            tn.isnewTop = true; // publish even if only qty change
            if (bids[0].quantity == 0)
                bids.erase(bids.begin());

            return tn;
        }
    }
    return TradeNotification();
}

bool OrderBook::compareBids(const Order &a, const Order &b)
{
    if (a.price == b.price)
        return a.timestamp < b.timestamp;
    return a.price < b.price;
}

bool OrderBook::compareAsks(const Order &a, const Order &b)
{
    if (a.price == b.price)
        return a.timestamp < b.timestamp;
    return a.price > b.price;
}

template <typename Compare>
void OrderBook::insertSorted(std::vector<Order> &orders, const Order &order, Compare comp)
{
    auto it = orders.begin();
    while (it != orders.end() && comp(order, *it))
    {
        ++it;
    }
    orders.insert(it, order);
}

std::pair<bool, bool> OrderBook::cancelOrderInVector(std::vector<Order> &orders, const int &userId, const int &orderId)
{
    auto it = orders.begin();
    bool orderCanceled = false;
    bool wasFirstElement = false;

    while (it != orders.end())
    {
        if (it->userId == userId && it->orderId == orderId)
        {
            if (it == orders.begin())
            {
                wasFirstElement = true;
            }
            it = orders.erase(it);
            orderCanceled = true;
        }
        else
        {
            ++it;
        }
    }

    return std::make_pair(orderCanceled, wasFirstElement);
}