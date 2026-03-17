#include "order_book.h"
#include <iostream>
using namespace std;
OrderBook::OrderBook() : event_handler_(), memory_pool_(MAX_ORDERS), best_bid_price_(ZERO), best_ask_price_(MAX_PRICE_LEVELS) {
    buy_price_levels_.resize(MAX_PRICE_LEVELS+ONE);
    sell_price_levels_.resize(MAX_PRICE_LEVELS+ONE);
    orders_.resize(MAX_ORDERS);
    event_handler_.RunEventLoop();
}

OrderBook::~OrderBook(){
    event_handler_.StopEventLoop();
    // for(auto &order_node : orders_){
    //     if(order_node) delete order_node;
    // }
}

void OrderBook::ProcessOrder(Order order){

    switch (order.event_type) {
        case 'N': // New Order
            addOrder(order);
            break;
        case 'C': // Cancel Order
            cancelOrder(order);
            break;
        case 'M': // Modify Order
            modifyOrder(order);
            break;
    }
}

void OrderBook::addOrder(Order order){
    if (order.side == 'B') {
        addBuyOrder(order);
    } else {
        addSellOrder(order);
    }
}

void OrderBook::addBuyOrder(Order order){
    int initial_quantity = order.quantity;
    while(order.price >= best_ask_price_ && order.quantity > ZERO){
        auto sell_order_node = sell_price_levels_[best_ask_price_].head;
        int exec_quantity=min(order.quantity,sell_order_node->quantity);
        order.quantity-=exec_quantity;
        sell_order_node->quantity-=exec_quantity;
        sendExecEvent(order, exec_quantity, best_ask_price_, order.quantity == ZERO ? 'C' : 'P');
            
        if(updatePriceLevel(sell_price_levels_[best_ask_price_])){
            updateBestAsk();
        }
    }
    if(order.quantity!= ZERO){
        auto buy_node = memory_pool_.Allocate();
        buy_node->order_id = order.order_id;
        buy_node->quantity = order.quantity;
        buy_node-> next = nullptr;
        buy_node->prev = nullptr;
        buy_node->price_level=&buy_price_levels_[order.price];
        orders_[order.order_id] = buy_node;
        buy_price_levels_[order.price].AddOrder(buy_node);
        if(order.price > best_bid_price_)best_bid_price_=order.price;

    } 
    sendAddEvent(order, initial_quantity);


}
void OrderBook::addSellOrder(Order order){
    int initial_quantity = order.quantity;

    while(order.price <= best_bid_price_ && order.quantity > ZERO){
        auto buy_order_node = buy_price_levels_[best_bid_price_].head;
        int exec_quantity=min(order.quantity,buy_order_node->quantity);
        order.quantity-=exec_quantity;
        buy_order_node->quantity-=exec_quantity;
        sendExecEvent(order, exec_quantity, best_bid_price_, order.quantity == ZERO ? 'C' : 'P');
        if(buy_order_node->quantity == ZERO){
            if(updatePriceLevel(buy_price_levels_[best_bid_price_])){
                updateBestBid();
            }
        }
    }
    if(order.quantity){
        auto sell_node = memory_pool_.Allocate();
        sell_node->order_id = order.order_id;
        sell_node->quantity = order.quantity;
        sell_node-> next = nullptr;
        sell_node->prev = nullptr;
        sell_node->price_level=&sell_price_levels_[order.price];
        orders_[order.order_id] = sell_node;
        sell_price_levels_[order.price].AddOrder(sell_node);
        if(order.price < best_ask_price_)best_ask_price_=order.price;
    }
    sendAddEvent(order, initial_quantity);
}
void OrderBook::cancelOrder(Order order){
    auto order_node = orders_[order.order_id];
    if(order_node){    
        auto price_level = order_node->price_level;
        if(order_node->prev == nullptr){
            price_level->head=order_node->next;
            if(order_node->next) order_node->next->prev = nullptr;
        }else{
            order_node->prev->next=order_node->next;
        }
        if(order_node->next == nullptr){
            price_level->tail=order_node->prev;
            if(order_node->prev)order_node->prev->next = nullptr;
        }else{
            order_node->next->prev=order_node->prev;
        }
        orders_[order.order_id] = nullptr;
        if(updatePriceLevel(*(order_node->price_level))){
            if(order_node->price_level == &buy_price_levels_[order_node->price_level - &buy_price_levels_[ZERO]]){
                updateBestBid();
            } else {
                updateBestAsk();
            }
        }
        sendCancelEvent(order);
        memory_pool_.Deallocate(order_node);
    }


}
void OrderBook::modifyOrder(Order order){
    cancelOrder(order);
    addOrder(order);
}

void OrderBook::updateBestBid(){  
    while(best_bid_price_ > ZERO && buy_price_levels_[best_bid_price_].head == nullptr){
        best_bid_price_--;
    }
}

void OrderBook::updateBestAsk(){
    while(best_ask_price_ < MAX_PRICE_LEVELS && sell_price_levels_[best_ask_price_].head == nullptr){
        best_ask_price_++;
    }
}

bool OrderBook::updatePriceLevel(PriceLevel &price_level){
    if(price_level.head == nullptr){
        return true; // Price level is empty
    }
    if(price_level.head->quantity == ZERO){
        // Remove the order from the price level
        orders_[price_level.head->order_id] = nullptr; // Update reference to the new head order node
        if(price_level.head->next != nullptr){
            price_level.head = price_level.head->next;
            memory_pool_.Deallocate(price_level.head->prev);  // Free the memory of the old head order node
            price_level.head->prev = nullptr;
        } else {
            memory_pool_.Deallocate(price_level.head); // Free the memory of the old head order node
            price_level.head = nullptr;
            price_level.tail = nullptr;
            return true; // Price level is now empty
        }
    }
    return false; // Price level still has orders
}

void OrderBook::sendAddEvent(Order order, int initial_quantity){
    if(initial_quantity != order.quantity){
        Event event{
            .order_id = order.order_id,
            .quantity = initial_quantity - order.quantity,
            .price = order.price,
            .side = order.side,
            .event_type = 'E',
            .status = order.quantity == ZERO ? 'C' : 'P'
        };
        event_handler_.HandleEvent(event);
    }
}

void OrderBook::sendExecEvent(Order order, int exec_quantity, int exec_price, char status){
    Event event{
        .order_id = order.order_id,
        .quantity = exec_quantity,
        .price = exec_price,
        .side = order.side,
        .event_type = 'E',
        .status = status
    };
    event_handler_.HandleEvent(event);
}

void OrderBook::sendCancelEvent(Order order){
    Event event{
        .order_id = order.order_id,
        .quantity = order.quantity,
        .price = order.price,
        .side = order.side,
        .event_type = 'C',
        .status = 'X'
    };
    event_handler_.HandleEvent(event);
}