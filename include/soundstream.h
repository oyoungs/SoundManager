#ifndef SOUND_STREAM_H
#define SOUND_STREAM_H

#include <memory>

typedef unsigned long long ulonglong;
class SoundStream
{
public:
    virtual void open() = 0;
    virtual void close() = 0;
    virtual ulonglong pos() const = 0;
    virtual ulonglong total() const = 0;
    virtual void setPos(ulonglong pos) = 0;
    virtual unsigned int channels() const = 0;
    virtual unsigned int sample_rate() const = 0;
    virtual unsigned int byte_rate() const = 0;  // = sample_rate * channels * bits_per_sample / 8
    virtual unsigned int block_align() const = 0; // = channels * bits_persample / 8
    virtual unsigned int bits_per_sample() const = 0;
    virtual int read(void *data, int length) = 0;
};

class StreamCreator {
public:
    static std::shared_ptr<SoundStream> createSoundStream(const std::string& file);
};

#endif /* SOUND_STREAM_H */
