#pragma once

#include "order_node.h"

/**
 * For a given price level we maintain a linked list of orders at that price level. The head of the list is the oldest order and the tail is the newest order.
 */
class PriceLevel {
    public:

        PriceLevel();
        inline void AddOrder(OrderNode* order_node){
            if(head == nullptr){
                head = order_node;
                tail = head;
            } else {
                tail->next = order_node;
                order_node->prev = tail;
                tail = order_node;
            }
        }

        OrderNode* head;
        OrderNode* tail;
        
};

