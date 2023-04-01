#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
  const TCPHeader header = seg.header();
  uint64_t abs_seqno;
  if(header.syn) {
    syn = true;
    isn=header.seqno;
  } 

  if(syn){
    abs_seqno = unwrap(header.seqno, isn, stream_out().bytes_written()+1);
    if(header.syn && (header.fin || seg.payload().size()>0))
      abs_seqno++;
    if(abs_seqno > 0) 
      _reassembler.push_substring( seg.payload().copy(), abs_seqno - 1 , header.fin);
  } 
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
  if(!syn )
    return std::nullopt;
  return wrap(stream_out().bytes_written()+1 + (stream_out().input_ended() ? 1 : 0), isn); 
}

size_t TCPReceiver::window_size() const { 
  return stream_out().remaining_capacity(); 
}
