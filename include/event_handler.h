#pragma once


#include "event.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <atomic>


inline constexpr int MAX_BUFFER_SIZE = (1<<16); // Assuming a maximum of 1 million events in the queue
inline constexpr int STRING_BUFFER_SIZE = (1<<20); // Buffer size for string concatenation before printing to avoid excessive I/O operations
inline constexpr int STRING_BUFFER_DELTA = (1<<10); // Maximum buffer size to prevent excessive memory usage in case of a large number of events
inline constexpr char SPACE_CHAR = ' '; // Character used to separate fields in the output string
/**
  * Handles Events Emitted by the OrderBook class
*/
class EventHandler{
    public:
        void HandleEvent(Event &event); // will be implemented later and called only when event is completed
        void RunEventLoop(); // will be implemented later to run the event loop and process events from a queue
        void StopEventLoop(); // will be implemented later to stop the event loop
    
    private:
        std::atomic<bool> is_event_loop_running_{false}; // flag to indicate when to stop the event loop
        std::thread event_loop_thread_; // thread to run the event loop
        std::array<Event, MAX_BUFFER_SIZE> event_buffer_; // buffer to store events before processing
        alignas(64) std::atomic<size_t> write_head_{0}; // index to keep track of the next event to be processed in the buffer
        alignas(64) std::atomic<size_t> read_head_{0}; // index to keep track of the next event to be processed in the buffer

        std::string string_buffer_; // thread local buffer to avoid false sharing between threads when writing events to the buffer 
};