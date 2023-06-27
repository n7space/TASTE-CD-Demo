#include "dataview-uniq.h"

#include <atomic>
#include <chrono>

#define QUEUE_SIZE 65536
#define QUEUE_PACKETS_MAX_COUNT 50

using namespace std::chrono_literals;

class arbiter_state {
public:
    /* Listening window (the window opened after sending a token to the second endpoint)
     * lasts through one tick of the Doorman cyclic interface. The "listening_interval"
     * is a sum of listening and sending window numbers.
     * Examples:
     * - number 2 means: 1 listening window per 1 sending window;
     * - number 5 means: 1 listening window per 4 sending windows;
     * - number 20 means: 1 listening window per 19 sending windows;
     * We can control the time of opening these windows using that value. If the
     * Doorman ticks every 250ms and listening_interval is set to 5, it means that
     * the listening window lasts for 250ms and the sending windows lasts for
     * 1000ms (250s * 4).
     */
    inline static const uint8_t listening_interval = 2;
    inline static const uint8_t open_listening_window_message[] = { 0xef };

    asn1SccPlatformSpecificPointer private_data;
    std::atomic_bool is_receiving;

    uint8_t queue_buffer[QUEUE_SIZE];
    size_t queue_packets_size[QUEUE_PACKETS_MAX_COUNT];
    size_t queue_packets_count;
    size_t queue_all_packets_size;
};
