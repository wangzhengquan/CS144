#include "wrapping_integers.hh"

#define MOD (1ul << 32)
// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
  return isn + uint32_t(n);
}



//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap1(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t absseq = static_cast<uint32_t>(n - isn) ; 
    //std::cout << "n=" <<  n << " isn="<<isn << " absseq=" << absseq << std::endl;
    if(absseq > checkpoint)
      return absseq;

    uint64_t check_mod = (checkpoint >> 32) << 32;
    if(check_mod > MOD)
      check_mod -= MOD;

    absseq |= check_mod;

    uint64_t nxt_absseq = absseq + MOD;
    while(labs(absseq - checkpoint) > labs(nxt_absseq - checkpoint)){
      absseq = nxt_absseq;
      nxt_absseq = absseq + MOD;
    }
     
    return absseq;
}

uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t absseq = static_cast<uint32_t>(n - isn) ; 
    uint64_t nxt_absseq;
    //std::cout << "n=" <<  n << " isn="<<isn << " absseq=" << absseq << std::endl;
    if(absseq > checkpoint)
      return absseq;
    
    absseq |= (checkpoint >> 32) << 32;

    if(absseq > checkpoint){
      nxt_absseq = absseq;
      absseq -=  MOD;
    } else {
       nxt_absseq = absseq + MOD;
    }
    
    if(checkpoint - absseq  > nxt_absseq - checkpoint){
      absseq = nxt_absseq;
    }
     
    return absseq;
}

 
 




