#include "reassembler.hh"
#include <cmath>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  if ( is_last_substring ) {
    eof_ = true;
    last_index_ = first_index + data.length();
  }

  uint64_t index = max( first_index, first_unpoped_index_ );
  uint64_t avail_cap = output_.writer().available_capacity();
  if ( first_index < first_unpoped_index_ + avail_cap && first_index + data.length() >= first_unpoped_index_ ) {

    avail_cap = first_unpoped_index_ + avail_cap - index;
    uint64_t len = data.length() - ( index - first_index );

    data = data.substr( index - first_index, min( avail_cap, len ) );
    if ( !data.empty() ) {
      if ( buf_.find( index ) != buf_.end() ) {
        if ( buf_[index].length() < data.length() )
          buf_[index] = data;
      } else
        buf_[index] = data;
      merge();
      index = buf_.begin()->first;
      string s = buf_.begin()->second;
      if ( (uint64_t)index <= first_unpoped_index_ ) {
        output_.writer().push( s );
        buf_.erase( index );
        first_unpoped_index_ += s.length();
      }
    } else
      merge();
  }

  if ( eof_ && last_index_ <= first_unpoped_index_ ) {
    output_.writer().close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  uint64_t res = 0;
  for ( auto [_, s] : buf_ ) {
    res += s.length();
  }
  return res;
}
