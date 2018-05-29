#ifndef CPP_LIKE_SWIFT_DEFER_H
#define CPP_LIKE_SWIFT_DEFER_H

#include <functional>
namespace swift {

    class Defer
    {
    public:
        Defer& operator+=(const std::function<void()> calllback) {
            _defer_callback = calllback;
        }

        ~Defer() {
            if(_defer_callback) {
                _defer_callback();
            }
        }

    private:
        std::function<void()> _defer_callback;
    };

}

#define defer swift::Defer __defer__; \
    __defer__ +=

#endif /* CPP_LIKE_SWIFT_DEFER_H */
