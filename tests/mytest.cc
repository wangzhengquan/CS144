#include "byte_stream.hh"
#include "string_conversions.hh"
#include "tcp_sender.hh"
#include "tcp_state.hh"
#include "util.hh"
#include "wrapping_integers.hh"

#include <algorithm>
#include <deque>
#include <exception>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <bitset>
#include <cstdint>
#include <stdexcept>
#include <queue>
#include <map>
#include "tcp_sponge_socket.hh"
 

int main(int argc, char *argv[]) {
    // uint64_t my_num = 6;

    // std::bitset<64> bits (my_num);
    // std::bitset<64> bits1(~my_num);
    // std::bitset<64> bits2(!my_num);
    // std::cout << bits << std::endl;
    // std::cout << bits1 << std::endl;
    // std::cout << bits2 << std::endl;

    // uint64_t a = 7, b=6;
    // uint64_t c = b-a;
    // std::cout << c << std::endl;
    // std::cout << UINT64_MAX << std::endl;
    // std::queue<TCPSegment> queue{};
    // TCPSegment seg{};
    // seg.header().seqno = WrappingInt32(15) ;
    // queue.push(seg);
    
    // std::cout << queue.front().header().summary() << std::endl;
    // queue.pop();
    // std::cout << queue.front().header().summary() << std::endl;
    // TCPSegment & ref = seg;
    // std::cout << ref.header().summary() << std::endl;
     

    std::map<int, std::string> mymap{{1, "hello"}};
    std::cout << mymap[1] << std::endl;
    std::cout << (mymap[0]=="") ;
    return 0;
}
