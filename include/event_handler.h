#pragma once


#include "event.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <atomic>

/**
  * Handles Events Emitted by the OrderBook class
*/
class EventHandler{
    public:
        void HandleEvent(Event event); // will be implemented later and called only when event is completed
        void RunEventLoop(); // will be implemented later to run the event loop and process events from a queue
        void StopEventLoop(); // will be implemented later to stop the event loop
    
    private:
        std::queue<Event> event_queue_; // will be used to store events emitted by the OrderBook class
        std::mutex queue_mutex_; // will be used to synchronize access to the event queue
        std::condition_variable event_cv_; // will be used to notify the event loop of new events
        std::atomic<bool> is_event_loop_running_{false}; // flag to indicate when to stop the event loop
        std::thread event_loop_thread_; // thread to run the event loop
};