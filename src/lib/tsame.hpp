#pragma once

template<typename A, typename B>
struct TypeIsSame {
    static constexpr bool value = false;
};

template<typename T>
struct TypeIsSame<T, T> {
    static constexpr bool value = true;
};
