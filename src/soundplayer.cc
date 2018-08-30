#include <oyoung/soundplayer.h>
#include <oyoung/soundstream.h>
#include "defer.h"
#include "soundpcm.h"

namespace oyoung {
    namespace audio {
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
                : _player_playing(false)
                , _player_paused(false)
                , _player_alive(true)
                , _player_should_play(false)
                , _player_name(name), _pcm(std::make_shared<snd::pcm::SoundPCM>()) {
            _player_play_thread = std::thread([=]{
                while (_player_alive) {
                    runBackgroundTask();
                }
            });
        }

        void SoundPlayer::load(std::shared_ptr<SoundStream> stream)  {

            if(is_playing()) stop();
            {
                std::lock_guard<std::mutex> lock(_raw_stream_mutext);
                _raw_stream = stream;
            }
        }

        std::shared_ptr<SoundStream> SoundPlayer::stream() const {
            std::lock_guard<std::mutex> lock(_raw_stream_mutext);
            return _raw_stream;
        }

        bool SoundPlayer::is_finished() const {
            return _raw_stream && _raw_stream->is_end();
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
            auto hw_params = _pcm->hw_params();

            if (!generateHwParams(hw_params)) {
                return false;
            }


            if(!_pcm->set_hw_params(hw_params)) {
                setErrorMsg( _pcm->error_msg());
                return false;
            }
            return true;
        }

        bool SoundPlayer::start() {

            if (_player_playing) {
                setErrorMsg("Player is Playing Now");
                return false;
            }

            if (stream() == nullptr) {
                setErrorMsg("Sound Stream Not Set");
                return false;
            }

            if(!stream()->is_open() && !stream()->open()) {
                setErrorMsg("Sound Stream Open Failed");
                return false;
            }

            if(!_pcm->is_open() && !_pcm->open(_player_name)) {
                setErrorMsg(_pcm->error_msg());
                return false;
            }


            if(!hw_params_configure()) {
                return false;
            }


            setShouldPlay(true);

            setPlayerPlaying(true);

            appendBackgroundTask([=] {
                try {
                    player_playing_on_new_thread();
                } catch (const std::exception& e) {
                    setErrorMsg(e.what());
                } catch (...) {
                    setErrorMsg("Unknow Exception");
                }
            });

            setErrorMsg("Success");

            return true;
        }

        void SoundPlayer::player_playing_on_new_thread() {

            if (stream() == nullptr) return;


            auto pcm_block_align = stream()->block_align();
            auto samples_per_second = stream()->sample_rate();
            auto size = _pcm->frames_to_bytes(samples_per_second);
            if (samples_per_second == 0) return;

            std::vector<char> buffer(size);
            long long time_per_sample = 1.0 / samples_per_second * 1000000; // single sample time: us

            _pcm->prepare();

            while (should_play()) {
                if (is_paused()) { // if paused, give the CPU time to the other threads
                    std::this_thread::sleep_for(
                            std::chrono::milliseconds(500));
                } else {
                    auto second_start = std::chrono::system_clock::now();
                    auto bytes = _raw_stream->read_pcm(buffer.data(), buffer.size());

                    if (bytes > 0) {
                        auto frames = _pcm->bytes_to_frames(bytes);
                        auto buffer_start = buffer.data();
                        do {
                            auto frame_start = std::chrono::system_clock::now();
                            auto frame = _pcm->writei(buffer_start, frames);

                            if (frame == -EPIPE) {
                                _pcm->prepare();
                            } else if (frame > 0) {

                                buffer_start += frame * pcm_block_align;
                                frames -= frame;

                                auto frame_stop = std::chrono::system_clock::now();
                                long long frame_left = time_per_sample * frame -
                                                       std::chrono::duration_cast<std::chrono::microseconds>(
                                                               frame_stop - frame_start).count();
                                if (frame_left > 0) {
                                    std::this_thread::sleep_for(std::chrono::microseconds(frame_left));
                                }
                            }

                        } while (should_play() && frames > 0);

                    }

                    if (!should_play()
                        || stream() == nullptr
                        || stream()->is_end()) {
                        setShouldPlay(false);
                        break;
                    }

                    auto second_stop = std::chrono::system_clock::now();
                    auto left_milli = 1000 - std::chrono::duration_cast<std::chrono::milliseconds>(
                            second_stop - second_start).count();
                    if (should_play() && left_milli > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(left_milli));
                    }
                }
            }

            _pcm->close();
            setPlayerPlaying(false);
            setPlayerPause(false);
        }

        void SoundPlayer::stop() {
            std::lock_guard<std::mutex> lock(_variant_mutex);
            _player_should_play = false;
        }

        void SoundPlayer::pause() {
            std::lock_guard<std::mutex> lock(_variant_mutex);
            if (_raw_stream) _player_paused = true;
        }

        void SoundPlayer::resume() {
            std::lock_guard<std::mutex> lock(_variant_mutex);
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
                setErrorMsg("Sound Stream Not Set");
                return false;
            }

            int dir = 0;
            auto channels = _raw_stream->channels();
            auto sample_rate = _raw_stream->sample_rate();
            auto format = _raw_stream->bits_per_sample();

            if (!hw.set_access(snd::pcm::ACCESS_RW_INTERLEAVED)) {
                setErrorMsg(hw.error_msg());
                return false;
            }
            if (!hw.set_channels(channels)) {
                setErrorMsg(hw.error_msg());
                return false;
            }
            if (!hw.set_format(format == 8 ? snd::pcm::FORMAT_U8 : snd::pcm::FORMAT_S16_LE)) {
                setErrorMsg(hw.error_msg());
                return false;
            }
            if (!hw.set_rate_near(&sample_rate, &dir)) {
                setErrorMsg(hw.error_msg());
                return false;
            }
            unsigned int buffer_time = 50000;
            if (!hw.set_buffer_time_near(&buffer_time, &dir)) {
                setErrorMsg(hw.error_msg());
                return false;
            }
            unsigned long buffer_size = _raw_stream->byte_rate();
            if (!hw.set_buffer_size_near(&buffer_size)) {
                setErrorMsg(hw.error_msg());
                return false;
            }


            return true;
        }


        SoundPlayer::~SoundPlayer() {
            _player_alive = false;
            _player_should_play = false;
            _background_task_cv.notify_all();
            if(_player_play_thread.joinable()) {
                _player_play_thread.join();
            }
        }

        void SoundPlayer::appendBackgroundTask(const std::function<void()> &task) {
            std::unique_lock<std::mutex> lock(_background_task_mutex);
            _background_task_queue.push(task);
            _background_task_cv.notify_all();
        }

        void SoundPlayer::runBackgroundTask() {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(_background_task_mutex);
                _background_task_cv.wait(lock, [=]{
                    return !_player_alive || !_background_task_queue.empty();
                });
                if(!_background_task_queue.empty() ) {
                    task = _background_task_queue.front();
                    _background_task_queue.pop();
                }
            }
            if(task) task();

        }

        bool SoundPlayer::should_play() const {
            return _player_should_play;
        }

        void SoundPlayer::setErrorMsg(const std::string &msg) {
            std::lock_guard<std::mutex> lock(_variant_mutex);
            _player_err_msg = msg;
        }

        void SoundPlayer::setShouldPlay(bool yes) {
            _player_should_play = yes;
        }

        void SoundPlayer::setPlayerPlaying(bool yes) {
            std::lock_guard<std::mutex> lock(_variant_mutex);
            _player_playing = yes;
        }

        void SoundPlayer::setPlayerPause(bool pause) {
            std::lock_guard<std::mutex> lock(_variant_mutex);
            _player_paused = pause;
        }

    }
}
