#pragma once
#include <terminal/terminal.hpp>

#include <mem/alloc.hpp>

#include <gen/err.hpp>

#include <initializer_list.hpp>

template<typename T>
struct Vector {
    static constexpr size_t BASE_VECTOR_SIZE = 5;
   
private:
    T* arr;
    u32 len;
    u32 capacity;
public:
    Vector() {
        len = 0;
        capacity = BASE_VECTOR_SIZE;

        arr = (T*) KernelAllocator::alloc(BASE_VECTOR_SIZE * sizeof(T));
    }
    Vector(std::initializer_list<T> list) : Vector() {
        for (const T& item : list) {
            pushBack(item);
        }
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
            if (!newArr) kpanic("New array allocation failed in Vector.");
            capacity *= 2;
           
            for (size_t i = 0; i < len; i++) {
                newArr[i] = arr[i];
            }

            KernelAllocator::free(arr);
            arr = newArr;
        }

        arr[len++] = elem;
    }
    auto operator=(const Vector& other) -> Vector& {
        if (this == &other) return *this;
        KernelAllocator::free(arr);
        len = other.len;
        capacity = other.capacity;
        arr = (T*) KernelAllocator::alloc(capacity * sizeof(T));
        for (size_t i = 0; i < len; i++) arr[i] = other.arr[i];
        return *this;
    }
    auto popBack() -> void {
        if (len != 0)
            len--;
    }
    auto eraseAt(size_t ind) -> void {
        if (ind >= len) return;

        for (size_t i = ind; i < len; i++) {
            arr[i] = arr[i + 1];
        }

        len--;
    }

    auto size() const -> size_t {
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
