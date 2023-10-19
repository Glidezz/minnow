#include <stdint.h>

class Timer
{
private:
  uint64_t initial_RTO_ms;
  uint64_t current_RTO_ms;
  uint64_t time_ms { 0 };
  bool running { false };

public:
  explicit Timer( uint64_t init_ROT ) : initial_RTO_ms( init_ROT ), current_RTO_ms( init_ROT ) {}

  void stop() { running = false; }

  void start()
  {
    time_ms = 0;
    running = true;
  }

  void tick( uint64_t ms_since_last_tick )
  {
    if ( running ) {
      time_ms += ms_since_last_tick;
    }
  }

  bool is_running() { return running; }

  bool is_expired() { return running && time_ms >= current_RTO_ms; }

  void doubleRTO() { current_RTO_ms *= 2; }

  void resetRTO() { current_RTO_ms = initial_RTO_ms; }
};
