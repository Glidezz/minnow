#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity )
{
  // buf_.resize( capacity_ + 1 );
}

bool Writer::is_closed() const
{
  // Your code here.
  return eof_;
}

void Writer::push( string data )
{
  // Your code here.
  if ( available_capacity() == 0 || data.empty() ) {
    return;
  }
  uint64_t n = min( data.length(), available_capacity() );
  // for ( uint64_t i = 0; i < len; i++ ) {
  //   buf_[( head++ ) % ( capacity_ + 1 )] = data[i];
  // }

  if ( n < data.size() ) {
    data = data.substr( 0, n );
  }
  data_queue_.push_back( std::move( data ) );
  view_queue_.emplace_back( data_queue_.back().c_str(), n );
  bytepushed_ += n;
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
  // if ( head != tail ) {
  //   return string_view( { &buf_[tail], 1 } );
  // } else
  //   return {};
  if ( !view_queue_.empty() ) {
    return view_queue_.front();
  } else
    return {};
}

void Reader::pop( uint64_t len )
{
  auto n = min( len, bytes_buffered() );
  while ( n > 0 ) {
    auto sz = view_queue_.front().size();
    if ( n < sz ) {
      view_queue_.front().remove_prefix( n );
      bytepoped_ += n;
      return;
    }
    view_queue_.pop_front();
    data_queue_.pop_front();
    n -= sz;
    bytepoped_ += sz;
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return bytepushed_ - bytepoped_;
}
