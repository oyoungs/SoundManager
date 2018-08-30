#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include "soundplayer.h"
#include "soundmixer.h"
#include "soundrecorder.h"

#include <memory>
namespace oyoung {
    namespace audio {


        class SoundManager {
        public:
            static std::shared_ptr<SoundPlayer> defaultPlayer();

        };
    }
}

#endif /* SOUND_MANAGER_H */
