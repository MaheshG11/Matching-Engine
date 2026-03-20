#include "order_book.h"
#include <iostream>
#include <chrono>
using namespace std;    
static uint64_t rng_state = 88172645463325252ull;

inline uint64_t xorshift() {
    uint64_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng_state = x;
    return x;
}

vector<Order> WorkloadGenerator(uint64_t num_workload_orders) {
    Order o;
    std::vector<Order> workload;
    std::vector<uint64_t> active_orders;
    uint64_t next_id = 1;

    for(uint64_t i=0;i<num_workload_orders;i++) {

        uint64_t r = xorshift();
        int event = r % 10;

        if(event < 7 || active_orders.empty()) {
            // NEW ORDER
            o.order_id = next_id++;
            o.side = (r & 1) ? 'B' : 'S';
            o.price = 10000 + (r % 2000);
            o.quantity = 1 + (r % 1000);
            o.event_type = 'N';

            active_orders.push_back(o.order_id);
        }
        else if(event < 9) {
            // CANCEL
            uint64_t idx = r % active_orders.size();
            o.order_id = active_orders[idx];
            o.event_type = 'C';

            active_orders[idx] = active_orders.back();
            active_orders.pop_back();
        }
        else {
            // MODIFY
            uint64_t idx = r % active_orders.size();
            o.order_id = active_orders[idx];
            o.quantity = 1 + (r % 1000);
            o.event_type = 'M';
        }

        workload.push_back(o);
    }
    return workload;
}

int main() {
    int num_workload_orders = 5'000'000;
    auto workload = WorkloadGenerator(num_workload_orders);
    vector<int> times_in_ns;
    times_in_ns.reserve(workload.size());

    {    
        OrderBook order_book;
        for(int i=0;i<(int)(workload.size()/5);i++) {
            order_book.ProcessOrder(workload[i]);
        }
        for(int i=workload.size()/5;i<(int)(workload.size());i++) {
            auto start = std::chrono::high_resolution_clock::now();
            order_book.ProcessOrder(workload[i]);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            times_in_ns.push_back(duration.count());
        }

    }
    sort(times_in_ns.begin(), times_in_ns.end());
    cout << "Best-case latency: " << times_in_ns.front() << " ns\n";
    cout << "P50 latency: " << times_in_ns[times_in_ns.size() * 50 / 100] << " ns\n";
    cout << "P90 latency: " << times_in_ns[times_in_ns.size() * 90 / 100] << " ns\n";
    cout << "P99 latency: " << times_in_ns[times_in_ns.size() * 99 / 100] << " ns\n";
    cout << "P99.9 latency: " << times_in_ns[times_in_ns.size() * 999 / 1000] << " ns\n";
    cout << "Worst-case latency: " << times_in_ns.back() << " ns\n";
    cout<<"Program finished\n";
    return 0;
}