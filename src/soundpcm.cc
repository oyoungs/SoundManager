#include "jdrv/soundpcm.h"
#include <alsa/asoundlib.h>

namespace snd {
namespace pcm {


SoundPCM::SoundPCM() : _raw_pcm(nullptr), _pcm_open(false), _pcm_error_msg() {
}

bool SoundPCM::open(const std::string& name, stream_t stream, int mode) {
    if(_pcm_open) {
        _pcm_error_msg = "pcm already open, you should close it before you reopen";
        return false;
    }
    _pcm_error_msg = snd_strerror(snd_pcm_open(&_raw_pcm, name.c_str(), (snd_pcm_stream_t)stream, mode));
    _pcm_open = _pcm_error_msg == "Success";
    return _pcm_open;
}

bool SoundPCM::is_open() const {
    return _pcm_open;
}

void SoundPCM::close() {
    if(_pcm_open) {
        _pcm_error_msg = snd_strerror(snd_pcm_close(_raw_pcm));
        _pcm_open = _pcm_error_msg != "Success"; // if _pcm_error_msg equals "Success", then it means closed
    }
}

HardwareParams SoundPCM::hw_params() {
    return HardwareParams(this);
}

std::string SoundPCM::name() const {
    return snd_pcm_name(_raw_pcm);
}

stream_t SoundPCM::stream() const {
    return (stream_t)snd_pcm_stream(_raw_pcm);
}


bool SoundPCM::prepare() {
    _pcm_error_msg = snd_strerror(snd_pcm_prepare(_raw_pcm));
    return _pcm_error_msg == "Success";
}

std::string SoundPCM::error_msg() const {
    return _pcm_error_msg;
}

bool SoundPCM::set_hw_params(HardwareParams &hw_params) {
    auto ret = snd_pcm_hw_params(_raw_pcm, hw_params._raw_hw_params);
    _pcm_error_msg = snd_strerror(ret);
    return _pcm_error_msg == "Success";
}

int SoundPCM::writei(const void *data, int length) {
    return snd_pcm_writei(_raw_pcm, data, length);
}

int SoundPCM::readi(void *data, int length) {
    return snd_pcm_readi(_raw_pcm, data, length);
}


int SoundPCM::bytes_to_frames(int bytes) {
    return snd_pcm_bytes_to_frames(_raw_pcm, bytes);
}

int SoundPCM::frames_to_bytes(int frames) {
    return snd_pcm_frames_to_bytes(_raw_pcm, frames);
}

SoundPCM::~SoundPCM() {
    close();
}



}
}
