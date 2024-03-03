#pragma once

#include <memory>
#include <optional>

#include "exception.hh"
#include "network_interface.hh"

// \brief A router that has multiple network interfaces and
// performs longest-prefix-match routing between them.
class Router
{
public:
  // Add an interface to the router
  // \param[in] interface an already-constructed network interface
  // \returns The index of the interface after it has been added to the router
  size_t add_interface( std::shared_ptr<NetworkInterface> interface )
  {
    _interfaces.push_back( notnull( "add_interface", std::move( interface ) ) );
    return _interfaces.size() - 1;
  }

  // Access an interface by index
  std::shared_ptr<NetworkInterface> interface( const size_t N ) { return _interfaces.at( N ); }

  // Add a route (a forwarding rule)
  void add_route( uint32_t route_prefix,
                  uint8_t prefix_length,
                  std::optional<Address> next_hop,
                  size_t interface_num );

  // Route packets between the interfaces
  void route();

private:
  // The router's collection of network interfaces
  std::vector<std::shared_ptr<NetworkInterface>> _interfaces {};
  struct item
  {
    uint32_t route_prefix;
    uint8_t prefix_length;
    std::optional<Address> next_hop;
    const size_t interface_num;
  };
  std::vector<item> router_table_ {};
  int match_length( uint32_t src_ip, uint32_t tgt_ip, uint8_t tgt_len )
  {
    if ( tgt_len == 0 )
      return 0;
    if ( tgt_len > 32 )
      return -1;

    uint8_t len = 32U - tgt_len;
    src_ip = src_ip >> len;
    tgt_ip = tgt_ip >> len;
    return ( src_ip == tgt_ip ) ? tgt_len : -1;
  }

  std::vector<item>::iterator longest_prefix_match_( uint32_t dst_ip )
  {
    int prefix = -1;
    auto res = router_table_.end();

    for ( auto it = router_table_.begin(); it != router_table_.end(); it++ ) {
      auto len = match_length( dst_ip, it->route_prefix, it->prefix_length );
      if ( len > prefix ) {
        res = it;
        prefix = len;
      }
    }
    return res;
  }
};
