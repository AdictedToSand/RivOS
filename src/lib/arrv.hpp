#pragma once
#include <gen/err.hpp>
#include <gen/vec.hpp>

#include <int.h>

template<typename T>
struct ArrayView {
private:
    T* arr;
    u32 len;
public:
    ArrayView(T* iarr, u32 ilen) : arr(iarr), len(ilen) {}

    ArrayView(Vector<T> v) : arr(v.raw()), len(v.size()) {}

    ArrayView(std::initializer_list<T> init) : arr(init.begin()), len(init.size()) {} 

    auto at(u32 ind) const -> Expected<T*> {
        if (ind >= len || !arr) {
            return ExpectedErr<T*>();
        } 

        return &arr[ind];
    }
    auto operator[](u32 ind) const -> Expected<T*> {
        return at(ind);
    }
    auto size() const -> u32 {
        return len;
    }

    auto begin() const -> T* { return arr; }
    auto end() const -> T* { return arr + len; }
};
