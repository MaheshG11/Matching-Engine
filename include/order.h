#pragma once

/**
  *  Class used only to recieve order details from the input file and to create Price objects in the PriceLevel class.
  */
struct Order{
    int order_id;
    int price;
    int quantity;
    char side; // 'B' for buy, 'S' for sell
    char event_type; // 'N' for new, 'C' for cancel, 'M' for modify      
};