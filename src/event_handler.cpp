#include "event_handler.h"
#include <cassert>
void EventHandler::HandleEvent(Event &event){
    auto write_index = write_head_.load(std::memory_order_relaxed);
    auto read_index = read_head_.load(std::memory_order_acquire);

    while(write_index - read_index >= MAX_BUFFER_SIZE){
        read_index = read_head_.load(std::memory_order_acquire);
    }

    event_buffer_[write_index & (MAX_BUFFER_SIZE - 1)] = event;
    write_head_.store(write_index + 1, std::memory_order_release);
}

void EventHandler::RunEventLoop(){
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    string_buffer_.reserve(STRING_BUFFER_SIZE + STRING_BUFFER_DELTA); // Reserve space for the string buffer to avoid frequent reallocations
    StopEventLoop(); // Ensure any existing event loop is stopped before starting a new one
    is_event_loop_running_.store(true, std::memory_order_release);
    std::thread event_loop_thread([this](){
        while(is_event_loop_running_.load(std::memory_order_acquire)){
            auto read_index = read_head_.load(std::memory_order_relaxed);

            if(read_index == write_head_.load(std::memory_order_acquire)){
                // Empty Buffer
                continue;
            }
            // process event
            Event& event = event_buffer_[read_index & (MAX_BUFFER_SIZE - 1)];            
            read_head_.store(read_index + 1, std::memory_order_release);
            string_buffer_.append(std::to_string(event.order_id) + SPACE_CHAR + event.side + SPACE_CHAR + std::to_string(event.price) + SPACE_CHAR + std::to_string(event.quantity) + SPACE_CHAR + event.event_type + SPACE_CHAR + event.status + '\n');
            
            if(string_buffer_.size() > STRING_BUFFER_SIZE){
                size_t val = write(1, string_buffer_.data(), string_buffer_.size());
                assert(val == string_buffer_.size());
                string_buffer_.clear();
            }
        }
    });
    swap(event_loop_thread_, event_loop_thread);

} 

void EventHandler::StopEventLoop(){
    is_event_loop_running_.store(false, std::memory_order_release);
    if(event_loop_thread_.joinable()){
        event_loop_thread_.join();
    }
}