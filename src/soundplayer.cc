#include "jdrv/soundplayer.h"
#include "jdrv/soundstream.h"
#include "defer.h"


namespace jdrv {
    namespace jaudio {
        SoundStreamPos SoundStreamPos::fromSeconds(double seconds) {
            SoundStreamPos pos{};
            u64 second = seconds;
            pos.second = second % 60;
            pos.minute = second / 60;
            pos.hour = pos.minute / 60;
            pos.minute = pos.minute % 60;
            pos.millisec = (seconds - second) * 1000;
            return pos;
        }

        double SoundStreamPos::toSeconds() const {
            return hour * 3600ULL + minute * 60ULL + second + millisec / 1000.0;
        }


        SoundPlayer::SoundPlayer(const std::string &name)
                : _player_playing(false), _player_paused(false), _player_name(name) {

        }

        void SoundPlayer::load(const std::shared_ptr<SoundStream> &stream)  {
            if(stream && !stream->is_open()) {
                stream->open();
            }
            if(is_playing()) stop();
            _raw_stream = stream;
        }

        bool SoundPlayer::is_finished() const {
            return _raw_stream->is_end();
        }

        bool SoundPlayer::is_playing() const {
            return _player_playing;
        }

        bool SoundPlayer::is_paused() const {
            return _player_paused;
        }

        std::string SoundPlayer::error_msg() const {
            return _player_err_msg;
        }

        std::shared_ptr<SoundStream> SoundPlayer::save_stream() {
            if (_raw_stream) _stream_stack.push(_raw_stream);
            return _raw_stream;
        }

        void SoundPlayer::restore_stream() {
            if (!_stream_stack.empty()) {
                _raw_stream = _stream_stack.top();
                _stream_stack.pop();
            }
        }

        bool SoundPlayer::hw_params_configure() {
            auto hw_params = _pcm.hw_params();

            if (!generateHwParams(hw_params)) {
                return false;
            }


            if(!_pcm.set_hw_params(hw_params)) {
                _player_err_msg = _pcm.error_msg();
                return false;
            }
            return true;
        }

        bool SoundPlayer::start() {

            if (_player_playing) {
                _player_err_msg = "Play is Playing Now";
                return false;
            }

            if (_raw_stream == nullptr) {
                _player_err_msg = "Sound Stream Not Set";
                return false;
            }


            if (_player_play_thread.joinable()) {
                _player_play_thread.join();
//                _player_play_thread = std::thread();
            }



            if (!_pcm.is_open() && !_pcm.open(_player_name)) {
                _player_err_msg = _pcm.error_msg();
                return false;
            }


            if(!hw_params_configure()) {
                return false;
            }





            _player_playing = true;

            _player_play_thread = std::thread([=] {

                auto samples_per_second = _raw_stream->sample_rate(); // TODO
                auto size = _pcm.frames_to_bytes(samples_per_second);
                auto buffer = new char[size];
                defer [=] {
                    delete buffer;
                };

                ulonglong time_per_sample = 1.0 / _raw_stream->sample_rate() * 1000000; // single sample time: us

                if (_pcm.prepare()) {

                    while (is_playing()) {
                        if (is_paused()) std::this_thread::sleep_for(std::chrono::milliseconds(500));  // if paused, give the CPU time to the other threads
                        else {
                            auto start = std::chrono::system_clock::now();
                            auto bytes = _raw_stream->read_pcm(buffer, size);

                            if (bytes > 0) {
                                auto frames = _pcm.bytes_to_frames(bytes);
                                auto start = buffer;
                                do {
                                    auto frame_start = std::chrono::system_clock::now();
                                    auto frame = _pcm.writei(start, frames);
                                    if (frame > 0) {
                                        start += frame * _raw_stream->block_align();
                                        frames -= frame;
                                    }
                                    if (frame == -EPIPE) {
                                        _pcm.prepare();
                                    }
                                    auto frame_stop = std::chrono::system_clock::now();
                                    auto frame_left = time_per_sample * frame -
                                                std::chrono::duration_cast<std::chrono::microseconds>(
                                                        frame_stop - frame_start).count();
                                    if (frame_left > 0) {
                                        std::this_thread::sleep_for(std::chrono::microseconds(frame_left));
                                    }
                                } while (is_playing() && frames > 0);

                            }

                            if (!is_playing() || _raw_stream->is_end()) {

                                break;
                            }

                            auto stop = std::chrono::system_clock::now();
                            auto left_milli = 1000 - std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
                            if (left_milli > 0) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(left_milli));
                            }


                        }
                    }
                }

                _pcm.close();
                _player_playing = false;
                _player_paused = false;
            });

            _player_err_msg = "Success";

            return true;
        }

        void SoundPlayer::stop() {
            if (_player_playing) {
                _player_playing = false;
                _player_paused = false;
            }

        }

        void SoundPlayer::pause() {
            if (_raw_stream) _player_paused = true;
        }

        void SoundPlayer::resume() {
            if (_raw_stream) _player_paused = false;
        }

        double SoundPlayer::currentSecond() const {
            return _raw_stream ? _raw_stream->pos() : 0;
        }

        void SoundPlayer::setPlayPos(const SoundStreamPos &pos) {
            pause();
            if (_raw_stream) {
                _raw_stream->setPos(pos.toSeconds());
            }
            resume();
        }

        SoundStreamPos SoundPlayer::currentPos() const {
            return SoundStreamPos::fromSeconds(currentSecond());
        }

        double SoundPlayer::totalSeconds() const {
            return _raw_stream ? _raw_stream->total() : 0;
        }

        bool SoundPlayer::generateHwParams(snd::pcm::HardwareParams &hw) {
            if (_raw_stream == nullptr) {
                _player_err_msg = "Sound Stream Not Set";
                return false;
            }

            int dir = 0;
            auto channels = _raw_stream->channels();
            auto sample_rate = _raw_stream->sample_rate();
            auto format = _raw_stream->bits_per_sample();

            if (!hw.set_access(snd::pcm::ACCESS_RW_INTERLEAVED)) {
                _player_err_msg = hw.error_msg();
                return false;
            }
            if (!hw.set_channels(channels)) {
                _player_err_msg = hw.error_msg();
                return false;
            }
            if (!hw.set_format(format == 8 ? snd::pcm::FORMAT_U8 : snd::pcm::FORMAT_S16_LE)) {
                _player_err_msg = hw.error_msg();
                return false;
            }
            if (!hw.set_rate_near(&sample_rate, &dir)) {
                _player_err_msg = hw.error_msg();
                return false;
            }
            unsigned int buffer_time = 50000;
            if (!hw.set_buffer_time_near(&buffer_time, &dir)) {
                _player_err_msg = hw.error_msg();
                return false;
            }
            unsigned long buffer_size = _raw_stream->byte_rate();
            if (!hw.set_buffer_size_near(&buffer_size)) {
                _player_err_msg = hw.error_msg();
                return false;
            }


            return true;
        }


        SoundPlayer::~SoundPlayer() {
            stop();
            if(_player_play_thread.joinable()) {
                _player_play_thread.join();
            }
        }

    }
}
