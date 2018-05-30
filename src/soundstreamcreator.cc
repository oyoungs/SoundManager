#include "jdrv/soundstream.h"
#include "wavstream.h"
#include <algorithm>

namespace std {

    std::string tolower(const std::string& src) {
        std::string result(src);

        std::transform(src.begin(), src.end(), result.begin(), [=](char c) { return std::tolower(c); });

        return result;
    }

}
namespace jdrv {
    namespace jaudio {
        std::shared_ptr<SoundStream> StreamCreator::createSoundStream(const std::string &file) {
            auto lowerfilename = std::tolower(file);
            if (lowerfilename.find("wav") != lowerfilename.npos) {
                return std::make_shared<WavStream>(file);
            }

            return nullptr;
        }
    }
}
