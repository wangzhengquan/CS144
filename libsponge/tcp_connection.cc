#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPConnection::send_segs()  { 
    _sender.fill_window();
    while (!_sender.segments_out().empty()) {
        TCPSegment segment = _sender.segments_out().front();
        _sender.segments_out().pop();
        set_ack_and_windowsize(segment);
        _segments_out.push(segment);
    }

    if(is_inbound_ended() && is_outbound_ended_and_acked() && !_linger_after_streams_finish){
        _active = false;
    }
}

void TCPConnection::set_ack_and_windowsize(TCPSegment &segment) {
    // ask receiver for ack and window size
    optional<WrappingInt32> ackno = _receiver.ackno();
    if (ackno.has_value()) {
        segment.header().ack = true;
        segment.header().ackno = ackno.value();
    }
    segment.header().win = static_cast<uint16_t>(_receiver.window_size());
    return;
}

void TCPConnection::abort() {
    _sender.send_empty_segment();
    TCPSegment RSTSeg = _sender.segments_out().front();
    _sender.segments_out().pop();
    set_ack_and_windowsize(RSTSeg);
    RSTSeg.header().rst = true;
    _segments_out.push(RSTSeg);
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
}

// prereqs1 : The inbound stream has been fully assembled and has ended.
bool TCPConnection::is_inbound_ended() {
    return _receiver.unassembled_bytes() == 0 && _receiver.stream_out().input_ended();
}
// prereqs2 : The outbound stream has been ended by the local application and fully sent (including
// the fact that it ended, i.e. a segment with fin ) to the remote peer.
bool TCPConnection::is_outbound_ended() {
    return _sender.stream_in().eof()  ;
}
// prereqs3 : The outbound stream has been fully acknowledged by the remote peer.
bool TCPConnection::is_outbound_ended_and_acked() {
    //&& _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2
    return _sender.stream_in().eof()  && _sender.bytes_in_flight() == 0;
}

// ===================================================

size_t TCPConnection::remaining_outbound_capacity() const {
    return _sender.stream_in().remaining_capacity();
}

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if (!_active) {
       return;
    }
    // 重置连接时间
    _time_since_last_segment_received = 0;
    if (seg.header().rst) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        // kills the connection permanently
        _active = false;
        return;
    }
    _receiver.segment_received(seg);
    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }
    if(seg.length_in_sequence_space() > 0 ){
        send_segs();
    }
    if (_receiver.ackno().has_value() 
        and (seg.length_in_sequence_space() == 0) 
        and seg.header().seqno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();
    }

    if(is_inbound_ended() && !is_outbound_ended()){
        _linger_after_streams_finish = false;
    }
} 

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) { 
    size_t len = _sender.stream_in().write(data); 
    send_segs();
    return len;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { 
    _time_since_last_segment_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick); 

    if(is_inbound_ended() && is_outbound_ended_and_acked() 
        && _linger_after_streams_finish
        && (_time_since_last_segment_received >= 10 * _cfg.rt_timeout)){
        _active = false;
    }
    if(_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS){
        abort();
    }
}

void TCPConnection::end_input_stream() {
    
    _sender.stream_in().end_input();
    send_segs();
}

void TCPConnection::connect() {
    send_segs();
    _active = true;
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            abort();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
