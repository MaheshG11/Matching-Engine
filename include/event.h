#pragma once

struct Event{
        int order_id;
        int quantity;
        int price;
        char side; // 'B' for buy, 'S' for sell
        char event_type; // 'N' for new, 'C' for cancel, 'M' for modify
        char status; // 'C' for completed, 'P' for partial fill, 'X' for cancelled
        char pad; // padding to make the struct size a multiple of 4 bytes
};