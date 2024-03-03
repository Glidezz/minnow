#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // Your code here.
  router_table_.emplace_back( route_prefix, prefix_length, next_hop, interface_num );
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  // Your code here.
  for ( auto& interfaces : _interfaces ) {
    auto& datas = interfaces->datagrams_received();
    while ( !datas.empty() ) {
      auto data = datas.front();
      if ( data.header.ttl > 1 ) {
        data.header.ttl--;
        data.header.compute_checksum();
        uint32_t dst = data.header.dst;
        auto it = longest_prefix_match_( dst );
        if ( it != router_table_.end() ) {
          interface( it->interface_num )
            ->send_datagram( data, it->next_hop.value_or( Address::from_ipv4_numeric( dst ) ) );
        }
      }
      datas.pop();
    }
  }
}
