#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();
   
    if (auto search = _ethernet_address_map.find(next_hop_ip); search != _ethernet_address_map.end())
    {
        EthernetAddressEntry &entry = search->second;
        if(entry.valide && _ms_clock - entry.reply_time <=30000) {
            real_send_datagram(dgram, entry.eth_ddress);
            return;
        } else if(_ms_clock - entry.request_time <= 5000 ){
            queue_datagram(next_hop_ip, dgram);
            return;
        }
    } 

    queue_datagram(next_hop_ip, dgram);
    // arp request
    ARPMessage arp_message{};
    arp_message.opcode = ARPMessage::OPCODE_REQUEST;
    arp_message.sender_ethernet_address = _ethernet_address;
    arp_message.sender_ip_address = _ip_address.ipv4_numeric();
    //arp_message.target_ethernet_address
    arp_message.target_ip_address = next_hop_ip;
    EthernetFrame ethernet_frame{};
    ethernet_frame.header().type = EthernetHeader::TYPE_ARP;
    ethernet_frame.header().src = _ethernet_address;
    ethernet_frame.header().dst = ETHERNET_BROADCAST;
    ethernet_frame.payload() = arp_message.serialize();
    _frames_out.push(ethernet_frame);
    EthernetAddressEntry entry{};
    entry.valide = false;
    entry.request_time = _ms_clock;
    _ethernet_address_map.insert_or_assign(next_hop_ip, entry);
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if (frame.header().dst != _ethernet_address && frame.header().dst != ETHERNET_BROADCAST)
        return nullopt;
    if(frame.header().type == EthernetHeader::TYPE_IPv4){
        InternetDatagram dgram;
        if(dgram.parse(frame.payload()) == ParseResult::NoError){
            return dgram;
        } else {
           cerr << __LINE__ << ": InternetDatagram parse error"  << endl;
           return nullopt;
        }
    } else if(frame.header().type == EthernetHeader::TYPE_ARP){
        ARPMessage arp_message;
        if(arp_message.parse(frame.payload()) == ParseResult::NoError) {
            EthernetAddressEntry entry{};
            if (auto search = _ethernet_address_map.find(arp_message.sender_ip_address); search != _ethernet_address_map.end()) {
                entry = search->second;
            }
            entry.valide = true;
            entry.eth_ddress = arp_message.sender_ethernet_address;
            entry.reply_time = _ms_clock;
            _ethernet_address_map.insert_or_assign(arp_message.sender_ip_address, entry);
            
            send_pending_datagram(arp_message.sender_ip_address, arp_message.sender_ethernet_address);
            if(arp_message.opcode == ARPMessage::OPCODE_REQUEST
                && arp_message.target_ip_address == _ip_address.ipv4_numeric()) {
                // arp reply
                ARPMessage arp_message_reply{};
                arp_message_reply.opcode = ARPMessage::OPCODE_REPLY;
                arp_message_reply.sender_ethernet_address = _ethernet_address;
                arp_message_reply.sender_ip_address = _ip_address.ipv4_numeric();
                arp_message_reply.target_ethernet_address = arp_message.sender_ethernet_address;
                arp_message_reply.target_ip_address = arp_message.sender_ip_address;
                EthernetFrame ethernet_frame{};
                ethernet_frame.header().type = EthernetHeader::TYPE_ARP;
                ethernet_frame.header().src = _ethernet_address;
                ethernet_frame.header().dst = arp_message.sender_ethernet_address;
                ethernet_frame.payload() = arp_message_reply.serialize();
                _frames_out.push(std::move(ethernet_frame) );
            }
        } else {
           cerr << __LINE__ << ": InternetDatagram parse error"  << endl;
           
        }
        return nullopt; 
    }
    return nullopt; 
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) { 
    _ms_clock += ms_since_last_tick;
}

void NetworkInterface::real_send_datagram(const InternetDatagram &dgram, const EthernetAddress & next_hop_eth_ddress ) {
    EthernetFrame ethernet_frame{};
    ethernet_frame.header().type = EthernetHeader::TYPE_IPv4;
    ethernet_frame.header().src = _ethernet_address;
    ethernet_frame.header().dst = next_hop_eth_ddress;
    ethernet_frame.payload() = dgram.serialize();
    _frames_out.push(ethernet_frame);
}

void NetworkInterface::send_pending_datagram(uint32_t next_hop_ip, const EthernetAddress & next_hop_eth_ddress){
    if (auto search = _pending_map.find(next_hop_ip); search != _pending_map.end()){
        std::shared_ptr<std::deque<InternetDatagram> > pending_queue = search->second;
        for(const InternetDatagram &dgram : *pending_queue){
            real_send_datagram(dgram, next_hop_eth_ddress);
        }
        _pending_map.erase(search);
    }
}

void NetworkInterface::queue_datagram(const uint32_t next_hop_ip, const InternetDatagram &dgram) {
    std::shared_ptr<std::deque<InternetDatagram> > _pending_queue;
    if (auto search = _pending_map.find(next_hop_ip); search != _pending_map.end()){
        _pending_queue = search->second;
    } else {
        _pending_queue = std::shared_ptr<std::deque<InternetDatagram> >(new std::deque<InternetDatagram>{});
        _pending_map[next_hop_ip] = _pending_queue;
    }
    _pending_queue->push_back(dgram);
} 
