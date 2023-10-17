#include "tcp_receiver.hh"
#include <iostream>
using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  if ( message.SYN ) {
    SYN_set = true;
    zero_point = message.seqno;
  }
  if ( SYN_set && message.FIN ) {
    FIN_set = true;
  }

  if ( !SYN_set )
    return;

  uint64_t first_index = message.seqno.unwrap( zero_point, inbound_stream.bytes_pushed() ) + message.SYN;
  // cout << "first_index  " << first_index << " payload  " <<  message.payload.length()<< endl;
  reassembler.insert( first_index - 1, message.payload, FIN_set, inbound_stream );
  reassembler_in = reassembler.bytes_pending();
  if ( FIN_set && reassembler_in == 0 )
    inbound_stream.close();
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  uint16_t window_size
    = inbound_stream.available_capacity() > 0xffff ? 0xffff : inbound_stream.available_capacity();

  if ( !SYN_set ) {
    return { optional<Wrap32> {}, window_size };
  }

  bool def_end = false;
  if ( FIN_set && reassembler_in == 0 ) {
    def_end = true;
  }
  // cout << "zero_point  " << zero_point.unwrap(Wrap32(0),0) << endl;
  // cout << "bytes_pushed  " << inbound_stream.bytes_pushed() << "  window_size : " << window_size << endl;
  return TCPReceiverMessage { optional<Wrap32>( zero_point + inbound_stream.bytes_pushed() + SYN_set + def_end ),
                              window_size };
}
