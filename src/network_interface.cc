#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // Your code here.
  uint32_t ip_address = next_hop.ipv4_numeric();
  if ( ip2mac_.find( ip_address ) != ip2mac_.end() ) {
    // 转发ipv4数据报
    transmit( { { ip2mac_[ip_address].first, ethernet_address_, EthernetHeader::TYPE_IPv4 }, serialize( dgram ) } );
  } else {
    // 待发送的数据报
    IP_and_datagram_[ip_address].push_back( move( dgram ) );
    if ( arp_timer.find( ip_address ) == arp_timer.end() ) {
      // 发送广播arp帧 询问目的mac地址
      ARPMessage message;
      message.sender_ethernet_address = ethernet_address_;
      message.sender_ip_address = ip_address_.ipv4_numeric();
      // message.target_ethernet_address = ETHERNET_BROADCAST; ？？
      message.target_ip_address = ip_address;
      message.opcode = ARPMessage::OPCODE_REQUEST;

      transmit( { { ETHERNET_BROADCAST, ethernet_address_, EthernetHeader::TYPE_ARP }, serialize( message ) } );
      arp_timer[ip_address] = 0;
    }
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // Your code here.
  if ( frame.header.dst != ETHERNET_BROADCAST && frame.header.dst != ethernet_address_ ) {
    // 不是发送给自己的
    return;
  }

  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram data;
    if ( !parse( data, frame.payload ) )
      return;
    datagrams_received_.emplace( move( data ) );
  } else if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    // 收到arp报文
    ARPMessage message;
    if ( parse( message, frame.payload ) ) {
      ip2mac_[message.sender_ip_address] = { message.sender_ethernet_address, 0 };

      if ( message.opcode == ARPMessage::OPCODE_REPLY ) {
        // arp_timer[message.sender_ip_address] = 0;
        auto& datas = IP_and_datagram_[message.sender_ip_address];
        for ( auto data : datas ) {
          // send_datagram( data, Address::from_ipv4_numeric( message.sender_ip_address ) );
          transmit( { { message.sender_ethernet_address, ethernet_address_, EthernetHeader::TYPE_IPv4 },
                      serialize( data ) } );
        }
        // IP_and_datagram_[message.sender_ip_address] = {};
        IP_and_datagram_.erase( message.sender_ip_address );

      } else if ( message.opcode == ARPMessage::OPCODE_REQUEST ) {
        if ( message.target_ip_address == ip_address_.ipv4_numeric() ) {
          // 发送自己的mac地址
          ARPMessage arpmessage;
          arpmessage.sender_ethernet_address = ethernet_address_;
          arpmessage.sender_ip_address = ip_address_.ipv4_numeric();
          arpmessage.target_ethernet_address = message.sender_ethernet_address;
          arpmessage.target_ip_address = message.sender_ip_address;
          arpmessage.opcode = ARPMessage::OPCODE_REPLY;

          transmit( { { message.sender_ethernet_address, ethernet_address_, EthernetHeader::TYPE_ARP },
                      serialize( arpmessage ) } );
        }
      }
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  static const size_t IP_MAP_TTL = 30000;
  static const size_t ARP_TTL = 5000;

  for ( auto it = ip2mac_.begin(); it != ip2mac_.end(); ) {
    it->second.second += ms_since_last_tick;
    if ( it->second.second >= IP_MAP_TTL ) {
      it = ip2mac_.erase( it );
    } else {
      it++;
    }
  }

  for ( auto it = arp_timer.begin(); it != arp_timer.end(); ) {
    it->second += ms_since_last_tick;
    if ( it->second >= ARP_TTL ) {
      it = arp_timer.erase( it );
    } else
      it++;
  }
}
