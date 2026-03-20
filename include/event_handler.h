#pragma once


#include "event.h"
#include <queue>
#include <iostream>
#include <thread>
#include <atomic>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cassert>

inline constexpr int MAX_BUFFER_SIZE = (1<<16); // Assuming a maximum of 1 million events in the queue
inline constexpr size_t FILE_SIZE = 1 << 29;
inline constexpr int DISTANCE = 4;
inline constexpr int PREFETCH_DISTANCE = 8;
/**
  * Handles Events Emitted by the OrderBook class
*/
class EventHandler{
    public:
        EventHandler();
        ~EventHandler();
        void HandleEvent(Event &event); // will be implemented later and called only when event is completed
        void RunEventLoop(); // will be implemented later to run the event loop and process events from a queue
        void StopEventLoop(); // will be implemented later to stop the event loop
        void WriteEventsToFile(Event& event); // will be implemented later to write events to a file
    private:
        std::atomic<bool> is_event_loop_running_{false}; // flag to indicate when to stop the event loop
        std::thread event_loop_thread_; // thread to run the event loop
        std::array<Event, MAX_BUFFER_SIZE> event_buffer_; // buffer to store events before processing
        size_t write_index_ = 0; 
        alignas(64) std::atomic<size_t> write_head_{0}; // index to keep track of the next event to be processed in the buffer
        alignas(64) std::atomic<size_t> read_head_{0}; // index to keep track of the next event to be processed in the buffer
        size_t read_index_ = 0; 
        int fd;
        char* m_buffer;
        size_t offset = 0;
};