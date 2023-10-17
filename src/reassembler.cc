#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{

  // Your code here.
  if ( is_last_substring ) {
    last_index_ = first_index + data.length();
    end_ = true;
  }
  uint64_t avail_cap_ = output.available_capacity();

  for ( uint64_t index = max( first_index, unpop_index_ );
        index < first_index + data.length() && index < unpop_index_ + avail_cap_;
        index++ ) {
    buf_[index] = data[index - first_index];
  }

  uint64_t len = 0;
  string s = "";
  while ( buf_.count( unpop_index_ ) && avail_cap_ > 0 ) {
    s += buf_[unpop_index_];
    buf_.erase( unpop_index_ );
    len++;
    unpop_index_++;
    avail_cap_--;
  }

  output.push( s );
  if ( end_ && unpop_index_ >= last_index_ ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return buf_.size();
}
