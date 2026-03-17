#include "event_handler.h"

void EventHandler::HandleEvent(Event event){
    auto write_index = write_head.load(std::memory_order_relaxed);
    auto read_index = read_head.load(std::memory_order_acquire);

    while(write_index - read_index >= MAX_BUFFER_SIZE){
        // Buffer is full, wait for the event loop to process some events
        std::this_thread::yield();
        read_index = read_head.load(std::memory_order_acquire);
    }

    event_buffer_[write_index & (MAX_BUFFER_SIZE - 1)] = event;
    write_head.store(write_index + 1, std::memory_order_release);
}

void EventHandler::RunEventLoop(){
    StopEventLoop(); // Ensure any existing event loop is stopped before starting a new one
    is_event_loop_running_.store(true, std::memory_order_release);
    std::thread event_loop_thread([this](){
        while(is_event_loop_running_.load(std::memory_order_acquire)){
            auto read_index = read_head.load(std::memory_order_relaxed);

            if(read_index == write_head.load(std::memory_order_acquire)){
                // Empty Buffer
                std::this_thread::yield();
                continue;
            }
            // process event
            Event event = event_buffer_[read_index & (MAX_BUFFER_SIZE - 1)];
            read_head.store(read_index + 1, std::memory_order_release);
            // std::cout << "Event: order_id: " << event.order_id << " side: " << event.side << " price: " << event.price << " quantity: " << event.quantity << " event_type: " << event.event_type << " status: " << event.status <<'\n';
            // Process the event (for now we just print it)
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