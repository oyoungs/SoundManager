#include "wavstream.h"
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <sspdlog/sspdlog.h>


namespace oyoung {
    namespace audio {
        WavStream::WavStream(const std::string &file) : _wav_handle(-1), _file_path(file), _data_offset(0) {

        }

        bool WavStream::open() {
            _wav_handle = ::open(_file_path.c_str(), O_RDONLY);
            loadWavFormatInformation();
            return _wav_handle >= 0;
        }

        bool WavStream::is_open() const {
            return _wav_handle >= 0;
        }

        void WavStream::close() {
            if (_wav_handle >= 0) {
                ::close(_wav_handle);
                _wav_handle = -1;
            }
        }

        std::string WavStream::name() const {
            return _file_path;
        }

        void WavStream::loadWavFormatInformation() {
            int head_size = sizeof(_wav_head);
            char data_chunk[4] = {'d', 'a', 't', 'a'};


            if (::read(_wav_handle, &_wav_head, head_size) == head_size) {
                while (_wav_head.format.data_chunk.chunck != *(int *) data_chunk) {
                    lseek(_wav_handle, _wav_head.format.list_chunk.size, SEEK_CUR);
                    ::read(_wav_handle, &_wav_head.format.data_chunk, sizeof(_wav_head.format.data_chunk));
                }
                _data_offset = lseek(_wav_handle, 0, SEEK_CUR);
                _end_of_file = lseek(_wav_handle, 0, SEEK_END);
                lseek(_wav_handle, _data_offset, SEEK_SET);
            }
        }

        int WavStream::read_pcm(void *data, int length) {
            return ::read(_wav_handle, data, length);
        }

        double WavStream::total() const {
            std::cout << "bytes: " << _wav_head.format.data_chunk.size << ", bytes per second: "
                      << _wav_head.format.bytes_per_second << std::endl;
            return double(_wav_head.format.data_chunk.size) / _wav_head.format.bytes_per_second;
        }

        double WavStream::pos() const {
            return double(lseek(_wav_handle, 0, SEEK_CUR) - _data_offset) / _wav_head.format.bytes_per_second;
        }

        void WavStream::setPos(double pos) {
            ulonglong offset = _wav_head.format.bytes_per_second * pos;
            if (offset < _wav_head.format.data_chunk.size) {
                lseek(_wav_handle, _data_offset + offset, SEEK_SET);
            }
        }

        bool WavStream::is_end() const {
            return lseek(_wav_handle, 0, SEEK_CUR) == _end_of_file;
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
    }
}
