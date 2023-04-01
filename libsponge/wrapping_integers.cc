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
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) { return isn + static_cast<uint32_t>(n); }

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
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t above_absseq = static_cast<uint32_t>(n - isn);
    uint64_t below_absseq;
    // std::cout << "n=" <<  n << " isn="<<isn << " absseq=" << absseq << std::endl;
    if (above_absseq > checkpoint)
        return above_absseq;

    above_absseq |= (checkpoint >> 32) << 32;

    if (above_absseq < checkpoint) {
        below_absseq = above_absseq;
        above_absseq = above_absseq + MOD;
    } else {
        below_absseq = above_absseq - MOD;
    }

    if (checkpoint - below_absseq > above_absseq - checkpoint) {
        return above_absseq;
    }
    return below_absseq;
}
// 网络上发现的这个方法更好
uint64_t unwrap2(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    // 找到n和checkpoint之间的最小步数
    int32_t min_step = n - wrap(checkpoint, isn);
    // 将步数加到checkpoint上
    int64_t ret = checkpoint + min_step;
    // 如果反着走的话要加2^32
    return ret >= 0 ? static_cast<uint64_t>(ret) : ret + (1ul << 32);
}
