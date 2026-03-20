#include "event_handler.h"
#include <cassert>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <immintrin.h>

EventHandler::EventHandler() : m_buffer(nullptr) {
    fd = open("events.log", O_RDWR | O_CREAT | O_TRUNC, 0666);
    assert(fd != -1);
    int res = ftruncate(fd, FILE_SIZE);
    m_buffer = static_cast<char*> (mmap(
        nullptr,
        FILE_SIZE,
        PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    ));
    assert(m_buffer != MAP_FAILED);

    for(auto &event:event_buffer_){}
}

EventHandler::~EventHandler() {
    Event sentinel{};
    sentinel.order_id = -1;
    WriteEventsToFile(sentinel);

    msync(m_buffer, offset, MS_SYNC);
    munmap(m_buffer, FILE_SIZE);
    m_buffer = nullptr;
    close(fd);
}

void EventHandler::HandleEvent(Event &event){
    auto read_index = read_head_.load(std::memory_order_acquire);

    while(write_index_ - read_index >= MAX_BUFFER_SIZE){
        _mm_pause(); // Buffer is full, wait for the event loop to consume events
        read_index = read_head_.load(std::memory_order_acquire);
    }

    event_buffer_[write_index_ & (MAX_BUFFER_SIZE - 1)] = event;
    write_index_++;
    if(write_index_ & DISTANCE){
        __builtin_prefetch(&event_buffer_[(write_index_ + PREFETCH_DISTANCE) & (MAX_BUFFER_SIZE - 1)]); // Prefetch the next cache line to be written
        write_head_.store(write_index_, std::memory_order_release);
    }
}

void EventHandler::RunEventLoop(){

    StopEventLoop(); // Ensure any existing event loop is stopped before starting a new one
    is_event_loop_running_.store(true, std::memory_order_release);
    std::thread event_loop_thread([this](){
        while(true){
            auto write_index = write_head_.load(std::memory_order_acquire);
            if(write_index - read_index_   < DISTANCE){
                // Empty Buffer or Not Enough Events to Process
                if(!is_event_loop_running_.load(std::memory_order_acquire)){
                    break;
                }
                _mm_pause();
                write_index = write_head_.load(std::memory_order_acquire);
                continue;
            }
            __builtin_prefetch(&event_buffer_[(read_index_  + PREFETCH_DISTANCE)& (MAX_BUFFER_SIZE - 1)]); // Prefetch the next cache line to be written
            while(write_index > read_index_ ){
                // Process events in batches of DISTANCE
                Event& event = event_buffer_[read_index_ & (MAX_BUFFER_SIZE - 1)];
                read_index_++;
                WriteEventsToFile(event);
            }
            // process event
            read_head_.store(read_index_, std::memory_order_release);
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

void EventHandler::WriteEventsToFile(Event& event){
    assert(offset + sizeof(Event) <= FILE_SIZE);
    memcpy(m_buffer + offset, &event, sizeof(Event));
    offset += sizeof(Event);    
}
