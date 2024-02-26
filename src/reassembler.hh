#pragma once

#include "byte_stream.hh"
#include <iostream>
#include <map>
class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler( ByteStream&& output )
    : output_( std::move( output ) ), buf_(), first_unpoped_index_( 0 ), last_index_( 0 )
  {}

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

private:
  ByteStream output_; // the Reassembler writes to this ByteStream
  std::map<uint64_t, std::string> buf_;
  uint64_t first_unpoped_index_;
  uint64_t last_index_;
  bool eof_ {};

  void merge()
  {
    if ( buf_.empty() )
      return;
    std::map<uint64_t, std::string> temp;
    std::string s = buf_.begin()->second;
    uint64_t l = buf_.begin()->first;
    for ( auto [index, str] : buf_ ) {
      uint64_t r = l + s.length();
      if ( index > r ) {
        temp[l] = s;
        s = str;
        l = index;
      } else if ( index + str.length() <= r ) {
        continue;
      } else {
        str = str.substr( r - index );
        s += str;
      }
    }
    temp[l] = s;
    buf_ = temp;
  }
};
