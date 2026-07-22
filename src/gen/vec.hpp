#pragma once
#include <terminal/terminal.hpp>

#include <mem/alloc.hpp>

#include <gen/err.hpp>

template<typename T>
struct Vector {
    static constexpr size_t BASE_VECTOR_SIZE = 5;
   
private:
    T* arr;
    size_t len;
    size_t capacity;

public:
    Vector() {
        len = 0;
        capacity = BASE_VECTOR_SIZE;

        arr = (T*) KernelAllocator::alloc(BASE_VECTOR_SIZE * sizeof(T));
    }

    // Why the FUCK doesn't a reference work
    auto operator[](size_t ind) -> Expected<T> {
        if (!arr) {
            return Expected<T>(Expected<T>::ErrorTypes::Error); // ...
        }

        if (ind < len) {
            return Expected<T>(arr[ind]); // Do not ask.
        }

        return Expected<T>(Expected<T>::ErrorTypes::Error);
    }

    auto pushBack(T elem) -> void {
        if (len >= capacity) {
            T* newArr = (T*) KernelAllocator::alloc((capacity * 2) * sizeof(T));
            capacity *= 2;
           
            for (size_t i = 0; i < len; i++) {
                newArr[i] = arr[i];
            }

            KernelAllocator::free(arr);
            arr = newArr;
        }

        arr[len++] = elem;
    }
    auto popBack() -> void {
        if (len != 0)
            len--;
    }
    auto eraseAt(int ind) -> void {
        if (ind >= len) return;

        for (int i = ind; i < len; i++) {
            arr[i] = arr[i + 1];
        }

        len--;
    }

    auto size() -> size_t {
        return len;
    }

    ~Vector() {
        KernelAllocator::free(arr);
    }

    auto begin() -> T* { return arr; }
    auto end() -> T* { return arr + len; }

    auto begin() const -> const T* { return arr; }
    auto end() const -> const T* { return arr + len; }

    Vector(const Vector&) = delete;
};
