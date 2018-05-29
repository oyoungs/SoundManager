#include "wavstream.h"
#include <fcntl.h>
#include <unistd.h>





WavStream::WavStream(const std::string &file) : _wav_handle(-1), _file_path(file), _data_offset(0) {

}

void WavStream::open() {
    _wav_handle = ::open(_file_path.c_str(), O_RDONLY);
    loadWavFormatInformation();
}

void WavStream::close() {
    if(_wav_handle >= 0) {
        ::close(_wav_handle);
    }
}

void WavStream::loadWavFormatInformation() {
    int head_size = sizeof(_wav_head);
    char data_chunk[4] = {'d', 'a', 't', 'a'};



    if(::read(_wav_handle, &_wav_head, head_size) == head_size) {
        while(_wav_head.format.data_chunk.chunck != *(int *)data_chunk) {
            lseek(_wav_handle, _wav_head.format.list_chunk.size, SEEK_CUR);
            ::read(_wav_handle, &_wav_head.format.data_chunk, sizeof(_wav_head.format.data_chunk));
        }
        _data_offset = lseek(_wav_handle, 0, SEEK_CUR);
    }
}

int WavStream::read(void *data, int length) {
    return ::read(_wav_handle, data, length);
}

ulonglong WavStream::total() const {
    return _wav_head.format.data_chunk.size / _wav_head.format.bytes_per_second;
}

ulonglong WavStream::pos() const {
    return (lseek(_wav_handle, 0, SEEK_CUR) - _data_offset) / _wav_head.format.bytes_per_second;
}

void WavStream::setPos(ulonglong pos) {
    auto offset = _wav_head.format.bytes_per_second * pos;
    if(offset < _wav_head.format.data_chunk.size) {
        lseek(_wav_handle, _data_offset + offset, SEEK_SET);
    }
}

unsigned int WavStream::channels() const {
    return _wav_head.format.channels;
}

unsigned int WavStream::sample_rate() const {
    return _wav_head.format.sample_rate;
}

unsigned int WavStream::byte_rate() const {
    return _wav_head.format.byte_rate;
}

unsigned int WavStream::block_align() const {
    return _wav_head.format.block_align;
}

unsigned int WavStream::bits_per_sample() const {
    return _wav_head.format.bits_per_sample;
}
