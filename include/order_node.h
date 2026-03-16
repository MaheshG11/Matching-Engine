#pragma once
#include "price_level.h"

class PriceLevel;

struct OrderNode{
    int order_id;
    int quantity;
    OrderNode* next;
    OrderNode* prev;
    PriceLevel* price_level; // Pointer to the price level this order belongs to
};