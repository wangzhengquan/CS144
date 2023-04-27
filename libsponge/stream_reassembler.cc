#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity),  _buffer(capacity), _bitmap(capacity, false) {}

void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {
    uint64_t data_len = data.length();
    uint64_t _first_unassembled = _output.bytes_written();
    uint64_t end_index, start_index, data_start, output_start, copy_len, output_len;
    if (eof)
        _eof = true;

    if (data_len == 0) {
        goto label_end;
    }
    end_index = index + data_len - 1;
    if (end_index < _first_unassembled) {
        goto label_end;
    }
    start_index = index;
    if (index < _first_unassembled) {
        start_index = _first_unassembled;
    }

    data_start = start_index - index;
    output_start = start_index - _first_unassembled;

    copy_len = min(data_len - data_start, _output.remaining_capacity() - output_start);
    if (eof && (copy_len < data_len - data_start)) {
        _eof = false;
    }
    if (copy_len == 0) {
        goto label_end;
    }
    // size_t copy_end = data_start + copy_len ;
    // size_t output_end = output_start + copy_len ;
    for (uint64_t i = 0; i < copy_len; i++) {
        if (!_bitmap[output_start + i]) {
            _buffer[output_start + i] = data[data_start + i];
            _bitmap[output_start + i] = true;
            _unassembled_bytes++;
        }
    }

    output_len = 0;
    while (_bitmap.front()) {
        _bitmap.pop_front();
        _bitmap.push_back(false);
        output_len++;
    }
    if (output_len > 0) {
        _output.write(string(_buffer.begin(), _buffer.begin() + output_len));
        _buffer.erase(_buffer.begin(), _buffer.begin() + output_len);
        _buffer.insert(_buffer.end(), output_len, '\0');
        _unassembled_bytes -= output_len;
    }

label_end:

    if (_eof && _unassembled_bytes == 0) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _buffer.empty(); }
