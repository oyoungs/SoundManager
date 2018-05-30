#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include "soundplayer.h"
#include "soundmixer.h"
#include "soundrecorder.h"

#include <memory>
namespace jdrv {
    namespace jaudio {


        class SoundManager {
        public:
            static std::shared_ptr<SoundPlayer> defaultPlayer();

        };
    }
}

#endif /* SOUND_MANAGER_H */
