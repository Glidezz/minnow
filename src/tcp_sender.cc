#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return sequence_numbers_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // Your code here.
  // 1.达到最大传输字节数（窗口大小）
  // 2.仅传输中的序列号没了且window_size=0才需要发送假消息
  if ( ( window_size_ && sequence_numbers_in_flight_ >= window_size_ )
       || ( !window_size_ && sequence_numbers_in_flight_ >= 1 ) )
    return;

  auto seqno = Wrap32::wrap( abs_ackno_, isn_ );

  auto window_size
    = window_size_ == 0 ? 1 : window_size_ - sequence_numbers_in_flight_ - (uint16_t)( seqno == isn_ );

  // 从bytestream中取出window_size个字符加入到mess中
  string mess;
  while ( reader().bytes_buffered() && mess.length() < window_size ) {
    auto str = reader().peek();
    str = str.substr( 0, window_size - mess.length() );
    mess += str;
    input_.reader().pop( str.length() );
  }

  while ( !mess.empty() || seqno == isn_ || ( !FIN_ && writer().is_closed() ) ) {
    auto len = min( mess.length(), TCPConfig::MAX_PAYLOAD_SIZE );

    string payload( mess.substr( 0, len ) );
    TCPSenderMessage message { seqno, seqno == isn_, move( payload ), false, writer().has_error() };
    // 没有信息要发送
    if ( !FIN_ && writer().is_closed() && len == mess.length()
         && ( sequence_numbers_in_flight_ + message.sequence_length() < window_size_
              || ( window_size_ == 0 && message.sequence_length() == 0 ) ) ) {
      FIN_ = message.FIN = true;
    }

    transmit( message );

    abs_ackno_ += message.sequence_length();
    sequence_numbers_in_flight_ += message.sequence_length();
    msg_queue_.push( message );

    if ( !FIN_ && writer().is_closed() && len == mess.size() ) {
      break;
    }

    seqno = Wrap32::wrap( abs_ackno_, isn_ );
    mess = mess.substr( len );
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  return { Wrap32::wrap( abs_ackno_, isn_ ), false, "", false, writer().has_error() };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  if ( msg.RST ) {
    writer().set_error();
    return;
  }

  window_size_ = msg.window_size;
  uint64_t abs_seq_k = msg.ackno ? msg.ackno.value().unwrap( isn_, abs_old_ackno_ ) : 0;

  if ( abs_seq_k > abs_old_ackno_ && abs_seq_k <= abs_ackno_ ) {
    abs_old_ackno_ = abs_seq_k;

    // 计时器清0
    timer_.resetRTO();
    timer_.start();
    consecutive_retransmissions_ = 0;

    while ( !msg_queue_.empty() ) {
      auto abs_seq = msg_queue_.front().seqno.unwrap( isn_, abs_old_ackno_ ) + msg_queue_.front().sequence_length();
      if ( abs_seq <= abs_seq_k ) {
        sequence_numbers_in_flight_ -= msg_queue_.front().sequence_length();
        msg_queue_.pop();
      } else
        break;
    }
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  if ( !msg_queue_.empty() ) {
    timer_.tick( ms_since_last_tick );
  }

  if ( timer_.is_expired() ) {
    transmit( msg_queue_.front() );
    if ( window_size_ > 0 ) {
      timer_.doubleRTO();
      consecutive_retransmissions_++;
    }
    timer_.start();
  }
}
