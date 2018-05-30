#include "jdrv/soundmanager.h"
namespace jdrv {
    namespace jaudio {
        std::shared_ptr<SoundPlayer> SoundManager::defaultPlayer() {
            return std::make_shared<SoundPlayer>("default");
        }
    }
}

