#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), buf_(), head( 0 ), tail( 0 ), bytepushed_( 0 ), bytepoped_( 0 )
{
  buf_.resize( capacity_ + 1 );
}

bool Writer::is_closed() const
{
  // Your code here.
  return eof_;
}

void Writer::push( string data )
{
  // Your code here.

  uint64_t len = min( data.length(), available_capacity() );
  for ( uint64_t i = 0; i < len; i++ ) {
    buf_[( head++ ) % ( capacity_ + 1 )] = data[i];
  }
  bytepushed_ += len;
}

void Writer::close()
{
  // Your code here.
  eof_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - ( bytepushed_ - bytepoped_ );
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytepushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return eof_ && ( bytepoped_ == bytepushed_ );
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytepoped_;
}

string_view Reader::peek() const
{
  // Your code here.
  if ( head != tail ) {
    return string_view( { &buf_[tail], 1 } );
  } else
    return {};
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  len = min( len, bytes_buffered() );
  tail = ( tail + len ) % ( capacity_ + 1 );
  bytepoped_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return bytepushed_ - bytepoped_;
}
