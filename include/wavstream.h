#ifndef SOUND_WAV_STREAM_H
#define SOUND_WAV_STREAM_H

#include "soundstream.h"

struct WavFormat
{
    unsigned int wave;  // "WAVE"

    unsigned int fmt_chunk; // "fmt "
    unsigned int fmt_size; // 16
    unsigned short audio_format; //  1: pcm, others: ...
    unsigned short channels; // 1: single, 2: double
    union {
        unsigned int sample_rate;
        unsigned int samples_per_second; //
    };
    union {
        unsigned int byte_rate;
        unsigned int bytes_per_second;
    };
    unsigned short block_align; // channels * bits_per_sample / 8
    unsigned short bits_per_sample;

    union {
        struct {
            unsigned int chunck; // "data"
            unsigned int size;
            unsigned char data[0];
        } data_chunk;
        struct {
            unsigned int chunck; // "LIST"
            unsigned int size;
            unsigned char data[0];
        } list_chunk;
    };

};

struct WavHead
{
    unsigned int riff; // "RIFF"
    unsigned int chunk_size;
    WavFormat format;
};


class WavStream: public SoundStream
{
public:
    WavStream(const std::string& file);

    void open() override;
    void close() override;
    ulonglong total() const override;
    ulonglong pos() const override;
    void setPos(ulonglong pos) override;

    unsigned int channels() const override;
    unsigned int sample_rate() const override;
    unsigned int byte_rate() const override;
    unsigned int block_align() const override;
    unsigned int bits_per_sample() const override;
    int read(void *data, int length) override;

private:
    void loadWavFormatInformation();

private:
    int _wav_handle;
    long long _data_offset;
    std::string _file_path;
    WavHead _wav_head;
};

#endif /* SOUND_WAV_STREAM_H */
