#include "oyoung/soundmanager.h"
namespace oyoung {
    namespace audio {
        std::shared_ptr<SoundPlayer> SoundManager::defaultPlayer() {
            return std::make_shared<SoundPlayer>("default");
        }
    }
}

