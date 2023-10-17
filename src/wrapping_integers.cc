#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // Convert absolute seqno → seqno
  return zero_point + n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // Convert seqno → absolute seqno
  /*
      this->raw_value_  :: a sequence number
      zero_point        :: Initial Sequence Number
      checkpoint        :: "the first unassembled index"
  */
  uint64_t offset = this->raw_value_ - zero_point.raw_value_;
  uint64_t temp = ( checkpoint & 0xffffffff00000000 ) + offset;
  uint64_t ret = temp;
  if ( abs( int64_t( temp + ( 1ul << 32 ) - checkpoint ) ) < abs( int64_t( temp - checkpoint ) ) ) {
    ret = ret + ( 1ul << 32 );
  }
  if ( temp >= ( 1ul << 32 )
       && abs( int64_t( temp - ( 1ul << 32 ) - checkpoint ) ) < abs( int64_t( ret - checkpoint ) ) ) {
    ret = temp - ( 1ul << 32 );
  }
  return ret;
}
