#ifndef SOUND_WAV_STREAM_H
#define SOUND_WAV_STREAM_H

#include <oyoung/soundstream.h>

#include <vector>
#include <functional>

namespace oyoung {
    namespace audio {

        struct guard {

            guard&operator+=(std::function<void()> operation);
            ~guard();

        private:

            std::vector<std::function<void()>> m_guard_operations;
        };
        
        struct WavFormat {
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

        struct WavHead {
            unsigned int riff; // "RIFF"
            unsigned int chunk_size;
            WavFormat format;
        };


        class WavStream : public SoundStream {
        public:
            WavStream(const std::string &file);

            bool open() override;

            bool is_open() const override;

            void close() override;

            std::string name() const override;

            double total() const override;

            double pos() const override;

            void setPos(double pos) override;

            bool is_end() const override;

            unsigned int channels() const override;

            unsigned int sample_rate() const override;

            unsigned int byte_rate() const override;

            unsigned int block_align() const override;

            unsigned int bits_per_sample() const override;

            int read_pcm(void *data, int length) override;

            ~WavStream() { close(); }
        private:
            void loadWavFormatInformation();

        private:
            int _wav_handle;
            long long _data_offset, _end_of_file;
            std::string _file_path;
            WavHead _wav_head;
        };
    }
}

#endif /* SOUND_WAV_STREAM_H */
