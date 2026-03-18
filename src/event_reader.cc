#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include "event.h"

int main() {
    int fd = open("events.log", O_RDONLY);
    assert(fd != -1);

    Event event;

    while (true) {
        ssize_t bytes = read(fd, &event, sizeof(Event));

        if (bytes == 0) break;                  // EOF
        assert(bytes == sizeof(Event));

        if (event.order_id == -1) break;        

        std::cout << "order_id: " << event.order_id
                  << " quantity: " << event.quantity
                  << " price: " << event.price
                  << " side: " << event.side
                  << " type: " << event.event_type
                  << " status: " << event.status
                  << "\n";
    }

    close(fd);
    return 0;
}