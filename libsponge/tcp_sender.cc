#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _ms_rto(retx_timeout)
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _next_seqno - _base; }

void TCPSender::fill_window() {
  TCPSegment seg{};
  size_t rwnd = _rwnd == 0 ? 1 : _rwnd;
  size_t outstanding_bytes = _next_seqno - _base;

  if(_closed || outstanding_bytes >= rwnd) {
    return;
  }
  if(_next_seqno==0){
    seg.header().syn = true;
    seg.header().seqno = wrap(_next_seqno, _isn); 
    _segments_out.push(seg);  
    _segments_outstanding.push(seg);
    _next_seqno += seg.length_in_sequence_space();
    if(!_timer_running){
      // start
      _timer_running = true;
      _ms_elapsed = 0;
    }
    return;
  }

  if(_stream.eof()) {
    seg.header().syn = false;
    seg.header().fin = true;
    seg.header().seqno = wrap(_next_seqno, _isn);
    seg.payload() =  Buffer();
    _segments_out.push(seg);  
    _segments_outstanding.push(seg);
    _next_seqno += seg.length_in_sequence_space();
    _closed = true;
    if(!_timer_running){
      // start
      _timer_running = true;
      _ms_elapsed = 0;
    }
    return;
  }

  while(!_stream.buffer_empty() 
    && (outstanding_bytes = _next_seqno - _base) < rwnd)
  {    
    seg.header().syn = false;
    seg.header().fin = false;
    seg.header().seqno = wrap(_next_seqno, _isn);   
    seg.payload() = _stream.read(min(rwnd - outstanding_bytes, TCPConfig::MAX_PAYLOAD_SIZE ));
    if(_stream.eof() && (rwnd - outstanding_bytes > seg.payload().size()) ) {
      seg.header().fin = true;
      _closed = true;
    }

    _segments_out.push(seg);  
    _segments_outstanding.push(seg);
    _next_seqno += seg.length_in_sequence_space();
    if(!_timer_running){
      // start
      _timer_running = true;
      _ms_elapsed = 0;
    }
  }

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
  uint64_t abs_ackno = unwrap(ackno, _isn, _base);
  if(abs_ackno < _base || abs_ackno > _next_seqno){
    return;
  } 
  if(abs_ackno == _base) {
    _rwnd = window_size;
    return;
  }
 // cout << "----- " << 0 << std::endl;
  while(not _segments_outstanding.empty()) {
    TCPSegment& seg = _segments_outstanding.front();
    if(unwrap(seg.header().seqno , _isn, _base) + seg.length_in_sequence_space()  <= abs_ackno){
    // cout << "----- " << 1 << std::endl;
      _segments_outstanding.pop();
      // cout << "----- " << 2 << std::endl;
    } else {
      break;
    }
  }
  
  _base = abs_ackno;
  _rwnd = window_size;

  _ms_rto = _initial_retransmission_timeout;
  if(bytes_in_flight() == 0){
    // stop
    _timer_running = false;
  } else {
    // restart
     _ms_elapsed = 0; 
  }
 
  _consecutive_retransmissions = 0;
  // cout << "----- " << 3 << std::endl;
  // fill_window();
  // cout << "----- " << 4 << std::endl;

}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
  if(!_timer_running)
    return;

  _ms_elapsed += ms_since_last_tick;
  if(_ms_elapsed < _ms_rto) return;

  _segments_out.push(_segments_outstanding.front());
   // restart
  _ms_elapsed = 0;

  if(_rwnd !=0){
    _consecutive_retransmissions++;
    _ms_rto *= 2;
  }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
  TCPSegment seg;
  seg.header().seqno = wrap(_next_seqno, _isn);
  _segments_out.push(seg);
}
