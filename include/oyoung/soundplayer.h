#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

#include <string>
#include <memory>
#include <thread>
#include <stack>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace snd {
namespace pcm {
    class SoundPCM;
    class HardwareParams;
}
}

typedef unsigned long long u64;
namespace oyoung {
    namespace audio {

        class SoundStream;

        struct SoundStreamPos {
            unsigned hour;
            unsigned minute;
            unsigned second;
            unsigned millisec;

            SoundStreamPos() : hour(0), minute(0), second(0), millisec(0) {}

            SoundStreamPos(unsigned h, unsigned m, unsigned s = 0, unsigned mi = 0) : hour(h), minute(m), second(s), millisec(mi) {}


            static SoundStreamPos fromSeconds(double seconds);

            double toSeconds() const;
        };

        class SoundPlayer {
        public:
            SoundPlayer(const std::string &name);

            void load(std::shared_ptr<SoundStream> stream);

            bool start();

            void pause();

            void resume();

            void stop();

            bool should_play() const;

            bool is_playing() const;

            bool is_paused() const;

            bool is_finished() const;

            std::string error_msg() const;

            std::shared_ptr<SoundStream> stream() const;



            void reset() {
                auto playing = is_playing();
                if (playing) stop();
                setPlayPos(0, 0, 0);
                if (playing) start();
            }

            std::shared_ptr<SoundStream> save_stream();

            void restore_stream();


            SoundStreamPos currentPos() const;

            double currentSecond() const;

            void setPlayPos(const SoundStreamPos &pos);

            void setPlayPos(unsigned h, unsigned m, unsigned s) { setPlayPos(SoundStreamPos(h, m, s)); }

            double totalSeconds() const;

            ~SoundPlayer();

        private:
            bool hw_params_configure();
            bool generateHwParams(snd::pcm::HardwareParams &hw);
            void player_playing_on_new_thread();
            void appendBackgroundTask(const std::function<void()>& task);
            void runBackgroundTask();

            void setErrorMsg(const std::string& msg);
        private:
            bool _player_paused;
            bool _player_playing;
            bool _player_alive;
            bool _player_should_play;
            std::string _player_name;
            std::string _player_err_msg;

            mutable std::mutex _raw_stream_mutext;
            std::shared_ptr<SoundStream> _raw_stream;

            std::thread _player_play_thread;
            std::mutex _background_task_mutex;
            std::mutex _variant_mutex;
            std::condition_variable _background_task_cv;
            std::queue<std::function<void()>> _background_task_queue;
            std::stack<std::shared_ptr<SoundStream>> _stream_stack;
            std::shared_ptr<snd::pcm::SoundPCM> _pcm;

            void setShouldPlay(bool yes);

            void setPlayerPlaying(bool yes);

            void setPlayerPause(bool pause);
        };
    }
}

#endif /* SOUND_PLAYER_H */
