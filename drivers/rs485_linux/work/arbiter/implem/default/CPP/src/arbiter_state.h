// Fill in this class with your context data (internal state):
// list all the variables you want global (per function instance)
#include "dataview-uniq.h"

#include <atomic>
#include <chrono>

#define QUEUE_SIZE 1000
#define QUEUE_PACKETS_MAX_COUNT 10

using namespace std::chrono_literals;

class arbiter_state {
public:
    inline static const std::chrono::milliseconds listening_window_time = 100ms;
    inline static const uint8_t open_listening_window_message[] = {
        0x18, 0xbb, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xaa, 0xc9, 0xf3
    };

    asn1SccPlatformSpecificPointer private_data;
    std::atomic_bool is_receiving;

    uint8_t queue_buffer[QUEUE_SIZE];
    size_t queue_packets_size[QUEUE_PACKETS_MAX_COUNT];
    size_t queue_packets_count;
    size_t queue_all_packets_size;
};
