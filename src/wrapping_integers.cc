#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.

  return zero_point + n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.

  uint32_t offset = raw_value_ - wrap( checkpoint, zero_point ).raw_value_;
  uint64_t result = checkpoint + offset;
  if ( offset > ( 1u << 31 ) && result >= ( 1ul << 32 ) )
    result -= ( 1ul << 32 );
  return result;
}
