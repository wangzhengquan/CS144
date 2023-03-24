#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t _capacity) : capacity(_capacity) {}

size_t ByteStream::write(const string &data) {
    size_t bytes_written = min(data.size(), remaining_capacity());
    buffer.insert(buffer.end(), data.begin(), data.begin() + bytes_written);
    total_bytes_written += bytes_written;
    return bytes_written;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t bytes_to_peek = min(len, buffer.size());
    return string(buffer.begin(), buffer.begin() + bytes_to_peek);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t bytes_to_pop = min(len, buffer.size());
    buffer.erase(buffer.begin(), buffer.begin() + bytes_to_pop);
    total_bytes_read += bytes_to_pop;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t bytes_to_read = min(len, buffer.size());
    string data(buffer.begin(), buffer.begin() + bytes_to_read);
    buffer.erase(buffer.begin(), buffer.begin() + bytes_to_read);
    total_bytes_read += bytes_to_read;
    return data;
}

void ByteStream::end_input() { _inputended = true; }

bool ByteStream::input_ended() const { return _inputended; }

size_t ByteStream::buffer_size() const { return buffer.size(); }

bool ByteStream::buffer_empty() const { return buffer.empty(); }

bool ByteStream::eof() const { return _inputended && buffer_empty(); }

size_t ByteStream::bytes_written() const { return total_bytes_written; }

size_t ByteStream::bytes_read() const { return total_bytes_read; }

size_t ByteStream::remaining_capacity() const { return capacity - buffer.size(); }
