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

  /*这段是受到清华大佬(huangrt01)博客的启发*/
  uint32_t offset = this->raw_value_ - wrap( checkpoint, zero_point ).raw_value_;
  uint64_t result = checkpoint + offset;
  /*
    如果新位置距离checkpoint的偏移offset大于1<<32的一半也就是1<<31,
    那么离checkpoint最近的应该是checkpoint前面的元素
    举个例子: 1---------7(checkpoint)----------------1<<32+1;
    由于是无符号数相减所以1-7 == 1<<32+1 - 7;
    所以应该是1距离7最近所以应该选1
  */
  if ( offset > ( 1u << 31 ) && result >= ( 1ul << 32 ) )
    result -= ( 1ul << 32 );
  return result;
}
