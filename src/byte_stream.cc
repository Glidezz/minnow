#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), error_( false ), end_( false ), poped_num_( 0 ), buf_(){}

void Writer::push( string data )
{
  // Your code here.
  uint64_t len = min( (uint64_t)data.length(), this->available_capacity() );
  for ( uint64_t i = 0; i < len; i++){
    buf_.push_back( data[i] );
  }
}

void Writer::close()
{
  // Your code here.
  this->end_ = true;
}

void Writer::set_error()
{
  // Your code here.
  this->error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return this->end_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - this->buf_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return poped_num_ + this->buf_.size();
}

string_view Reader::peek() const
{
  // Your code here.
  return string_view( { &buf_.front(), 1 } );
}

bool Reader::is_finished() const
{
  // Your code here.
  return this->end_ && this->buf_.size() == 0;
}

bool Reader::has_error() const
{
  // Your code here.
  return this->error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  len = min( len, this->bytes_buffered() );
  for ( uint64_t i = 0; i < len; i++){
    buf_.pop_front();
  }
  poped_num_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return this->buf_.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return poped_num_;
}
