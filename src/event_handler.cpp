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
    auto write_index = write_head_.load(std::memory_order_relaxed);
    auto read_index = read_head_.load(std::memory_order_acquire);

    while(write_index - read_index >= MAX_BUFFER_SIZE){
        _mm_pause(); // Buffer is full, wait for the event loop to consume events
        read_index = read_head_.load(std::memory_order_acquire);
    }

    event_buffer_[write_index & (MAX_BUFFER_SIZE - 1)] = event;
    write_head_.store(write_index + 1, std::memory_order_release);
}

void EventHandler::RunEventLoop(){

    StopEventLoop(); // Ensure any existing event loop is stopped before starting a new one
    is_event_loop_running_.store(true, std::memory_order_release);
    std::thread event_loop_thread([this](){
        while(is_event_loop_running_.load(std::memory_order_acquire)){
            auto read_index = read_head_.load(std::memory_order_relaxed);

            if(read_index == write_head_.load(std::memory_order_acquire)){
                // Empty Buffer
                _mm_pause();
                continue;
            }
            // process event
            Event& event = event_buffer_[read_index & (MAX_BUFFER_SIZE - 1)];            
            read_head_.store(read_index + 1, std::memory_order_release);
            WriteEventsToFile(event);
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
