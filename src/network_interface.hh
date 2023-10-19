#pragma once

#include "address.hh"
#include "ethernet_frame.hh"
#include "ipv4_datagram.hh"

#include <iostream>
#include <list>
#include <optional>
#include <queue>
#include <unordered_map>
#include <utility>

/*
连接IP（互联网层或网络层）与以太网（网络接入层或链路层）的“网络接口”。

  该模块是 TCP/IP 堆栈的最低层（将 IP 与较低层网络协议连接，例如以太网）。
   但同一个模块也可以作为路由器的一部分重复使用：路由器通常有许多网络接口，
   路由器的工作是在不同接口之间路由互联网数据报。

  网络接口将数据报（来自“客户”，例如 TCP/IP 堆栈或路由器）转换为以太网帧。
  为了填写以太网目标地址，它查找每个数据报的下一个 IP 跃点的以太网地址，
  并使用[地址解析协议](\ref rfc::rfc826) 发出请求。 在相反的方向上，网络接口接受以太网帧，
  检查它们是否是针对它的，如果是，则根据其类型处理有效负载。
  如果它是 IPv4 数据报，网络接口会将其向上传递到堆栈。 如果是 ARP
请求或回复，网络接口会处理该帧并根据需要进行学习或回复。
*/

// A "network interface" that connects IP (the internet layer, or network layer)
// with Ethernet (the network access layer, or link layer).

// This module is the lowest layer of a TCP/IP stack
// (connecting IP with the lower-layer network protocol,
// e.g. Ethernet). But the same module is also used repeatedly
// as part of a router: a router generally has many network
// interfaces, and the router's job is to route Internet datagrams
// between the different interfaces.

// The network interface translates datagrams (coming from the
// "customer," e.g. a TCP/IP stack or router) into Ethernet
// frames. To fill in the Ethernet destination address, it looks up
// the Ethernet address of the next IP hop of each datagram, making
// requests with the [Address Resolution Protocol](\ref rfc::rfc826).
// In the opposite direction, the network interface accepts Ethernet
// frames, checks if they are intended for it, and if so, processes
// the the payload depending on its type. If it's an IPv4 datagram,
// the network interface passes it up the stack. If it's an ARP
// request or reply, the network interface processes the frame
// and learns or replies as necessary.
class NetworkInterface
{
private:
  // Ethernet (known as hardware, network-access, or link-layer) address of the interface
  // Ethernet (known as hardware, network-access, or link-layer) address of the interface
  EthernetAddress ethernet_address_;

  // IP (known as Internet-layer or network-layer) address of the interface
  Address ip_address_;

  std::unordered_map<uint32_t, std::pair<EthernetAddress, size_t>> ip_and_mac_;
  std::unordered_map<uint32_t, size_t> arp_timer_;
  std::unordered_map<uint32_t, std::vector<InternetDatagram>> waited_dagrams_;
  std::queue<EthernetFrame> out_frames_;

public:
  // Construct a network interface with given Ethernet (network-access-layer) and IP (internet-layer)
  // addresses
  //使用给定的以太网（网络访问层）和 IP（互联网层）地址构建网络接口
  NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address );

  // Access queue of Ethernet frames awaiting transmission
  //等待传输的以太网帧的访问队列
  std::optional<EthernetFrame> maybe_send();

  /*
  发送封装在以太网帧中的 IPv4 数据报（如果知道以太网目标地址）。
  需要使用 [ARP](\ref rfc::rfc826) 查找下一跳的以太网目标地址。
  （“发送”是通过确保 Maybe_send() 在下次调用时释放帧来完成的，但请考虑帧一生成就发送。）
  */
  // Sends an IPv4 datagram, encapsulated in an Ethernet frame (if it knows the Ethernet destination
  // address). Will need to use [ARP](\ref rfc::rfc826) to look up the Ethernet destination address
  // for the next hop.
  // ("Sending" is accomplished by making sure maybe_send() will release the frame when next called,
  // but please consider the frame sent as soon as it is generated.)
  void send_datagram( const InternetDatagram& dgram, const Address& next_hop );

  /*
  接收以太网帧并做出适当响应。
  如果类型为 IPv4，则返回数据报。 如果类型是 ARP 请求，则从“发送者”字段中学习映射，并发送 ARP 回复。
  如果类型是 ARP 回复，则从“发送者”字段中了解映射。

  */
  // Receives an Ethernet frame and responds appropriately.
  // If type is IPv4, returns the datagram.
  // If type is ARP request, learn a mapping from the "sender" fields, and send an ARP reply.
  // If type is ARP reply, learn a mapping from the "sender" fields.
  std::optional<InternetDatagram> recv_frame( const EthernetFrame& frame );

  // Called periodically when time elapses
  void tick( size_t ms_since_last_tick );
};
