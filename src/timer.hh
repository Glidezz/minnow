#include <cstdint>
class timer
{
public:
  explicit timer( uint64_t init_ROT ) : initial_RTO_ms_( init_ROT ), current_RTO_ms_( init_ROT ) {}

  void start() { time_ms = 0; }

  void tick( uint64_t ms_since_last_tick ) { time_ms += ms_since_last_tick; }

  bool is_expired() { return time_ms >= current_RTO_ms_; }

  void doubleRTO() { current_RTO_ms_ *= 2; }

  void resetRTO() { current_RTO_ms_ = initial_RTO_ms_; }

private:
  uint64_t initial_RTO_ms_;
  uint64_t current_RTO_ms_;
  uint64_t time_ms { 0 };
};