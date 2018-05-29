#include "soundmanager.h"

std::shared_ptr<SoundPlayer> SoundManager::defaultPlayer() {
    return std::make_shared<SoundPlayer>("default");
}

