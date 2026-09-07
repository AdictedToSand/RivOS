#pragma once
#include <stddef.h>

#include <initializer_list.hpp>

namespace std {
    template<typename T, int N>
    struct array {
        private:
        T arr[N];
        size_t len;
        public:
        array(std::initializer_list<T> init) {
            size_t i = 0;
            for (const T& value : init) {
                if (i >= N)
                    break;
                arr[i++] = value;
            }
        }
        auto size() -> size_t {
            return len;
        }
        auto add(T elem) -> void {
            if (len + 1 >= N) {
                return; // Currently no good error handling:(
            }
            arr[len++] = elem;
        }
        auto at(size_t ind) -> T* {
            if (ind >= len) {
                return nullptr;
            }

            return &arr[ind];
        }
        auto operator[](size_t ind) -> T* {
            return at(ind);
        }
    };
}
