# 🚀 Low-Latency Matching Engine (C++)

A high-performance, in-memory matching engine designed with a focus on **low latency, determinism, and systems-level efficiency**.  


---

## ⚡ Overview

- Implements **price-time priority matching**
- Optimized for **nanosecond-level latency**
- Eliminates dynamic allocations in the hot path
- Uses **lock-free communication** and **mmap-based logging**

---

## 🧱 Architecture

- **Matching Engine Thread**
  - Processes orders (hot path)
  - No syscalls, no allocations

- **Event Handler Thread**
  - Consumes events via a **lock-free SPSC queue**
  - Writes events to a memory-mapped file

- **Memory Pool**
  - Preallocated storage for orders/events
  - Eliminates allocator overhead

---

## 📊 Performance

### Binary Logging (mmap + memcpy)

P50 latency: 64 ns  
P90 latency: 193 ns  
P99 latency: 425 ns  
P99.9 latency: 878 ns  
Worst-case latency: 25894 ns  


---

## 🔥 Key Optimizations

- **Memory Pool**
  - Removed per-order allocations

- **Lock-Free SPSC Queue**
  - Single producer / single consumer
  - No locks, minimal contention

- **mmap-based Logging**
  - Eliminated per-event syscalls
  - Replaced `write()` with direct memory writes (`memcpy`)

- **Binary Serialization**
  - Avoided string formatting overhead
  - Improved cache locality and reduced latency

---

## 📉 Latency Improvement

Initial implementation (with I/O): ~135 ms worst-case  
After mmap + binary logging: ~26 µs worst-case  

> Eliminated syscall overhead and scheduler-induced latency spikes.

---

## 📁 File Format

Events are written as fixed-size binary structs:

```cpp
struct Event {
    int order_id;
    int quantity;
    int price;
    char side;
    char event_type;
    char status;
    char pad;
};