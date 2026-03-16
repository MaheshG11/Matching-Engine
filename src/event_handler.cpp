#include "event_handler.h"

void EventHandler::HandleEvent(Event event){
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        event_queue_.push(event);
    }
    event_cv_.notify_one();
}

void EventHandler::RunEventLoop(){
    is_event_loop_running_ = true;
    std::thread event_loop_thread([this](){
        while(is_event_loop_running_){
            std::queue<Event> events_to_process;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                event_cv_.wait(lock, [this](){ return !event_queue_.empty() || !is_event_loop_running_; });
                swap(events_to_process, event_queue_);
            }
            while(events_to_process.size()){
                Event event = events_to_process.front();
                events_to_process.pop();
                std::cout << "Event: order_id: " << event.order_id << " side: " << event.side << " price: " << event.price << " quantity: " << event.quantity << " event_type: " << event.event_type << " status: " << event.status <<'\n';
            }
            std::cout<<std::endl;

            // Process the event (for now we just print it)
        }
    });
    swap(event_loop_thread_, event_loop_thread);

} 

void EventHandler::StopEventLoop(){
    is_event_loop_running_ = false;
    event_cv_.notify_all();
    if(event_loop_thread_.joinable()){
        event_loop_thread_.join();
    }
}

