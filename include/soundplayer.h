#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

#include <string>
#include <memory>
#include <thread>
#include <stack>

#include "soundpcm.h"

typedef unsigned long long u64;

class SoundStream;

struct SoundStreamPos
{
    unsigned hour;
    unsigned minute;
    unsigned second;

    SoundStreamPos() : hour(0), minute(0), second(0) {}
    SoundStreamPos(unsigned h, unsigned m, unsigned s) : hour(h), minute(m), second(s) {}


    static SoundStreamPos fromSeconds(u64 seconds);
    u64 toSeconds() const;
};

class SoundPlayer
{
public:
    SoundPlayer(const std::string& name);

    void load(const std::shared_ptr<SoundStream>& stream) {
        if(is_playing()) stop();
        save_stream();
        _raw_stream = stream;
        reset();
    }

    bool start();
    void pause();
    void resume();
    void stop();

    bool is_playing() const;
    bool is_paused() const;

    std::string error_msg() const;


    void reset() {
        auto playing = is_playing();
        if(playing) stop();
        setPlayPos(0, 0, 0);
        if(playing) start();
    }

    void save_stream();
    void restore_stream();


    SoundStreamPos currentPos() const;
    double currentSecond() const;
    void setPlayPos(const SoundStreamPos& pos);
    void setPlayPos(unsigned h, unsigned m, unsigned s) { setPlayPos(SoundStreamPos(h, m, s));}

    u64 totalSeconds() const;

    ~SoundPlayer();

private:
    bool generateHwParams(snd::pcm::HardwareParams& hw);

private:
    bool _player_paused;
    bool _player_playing;
    std::string _player_name;
    std::string _player_err_msg;
    std::shared_ptr<SoundStream> _raw_stream;
    std::thread _player_play_thread;

    std::stack<std::shared_ptr<SoundStream>> _stream_stack;
    snd::pcm::SoundPCM _pcm;

};

#endif /* SOUND_PLAYER_H */
