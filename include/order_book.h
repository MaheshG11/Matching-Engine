#pragma once
#include <vector>
#include <string>
#include "order.h"
#include "event_handler.h"
#include "price_level.h"
#include "order_node.h"

inline constexpr int MAX_PRICE_LEVELS = 100'000; // Assuming price levels range from 0 to 99999
inline constexpr int MAX_ORDERS = 5'000'000; // Assuming a maximum of 1 million orders in the order book
inline constexpr int ZERO = 0;
inline constexpr int N_ONE = -1;
inline constexpr int ONE = 1;

class OrderBook{
    public:
        OrderBook();
        ~OrderBook();
        void ProcessOrder(Order order);

    private:
        void addOrder(Order order);
        void addBuyOrder(Order order);
        void addSellOrder(Order order);
        void cancelOrder(Order order);
        void modifyOrder(Order order);
        void updateBestBid();
        void updateBestAsk();
        bool updatePriceLevel(PriceLevel &price_level);
        void sendAddEvent(Order order, int initial_quantity);
        void sendExecEvent(Order order, int exec_quantity, int exec_price, char status);
        void sendCancelEvent(Order order);

    private:
        EventHandler event_handler_;
        std::vector<PriceLevel> buy_price_levels_;
        std::vector<PriceLevel> sell_price_levels_;
        std::vector<OrderNode*> orders_; // to keep track of all orders in the order book
        int best_bid_price_;
        int best_ask_price_;

};