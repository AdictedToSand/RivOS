#pragma once
#include <gen/err.hpp>

template<typename T1, typename T2>
struct Sum {
private:
    T1 t1;
    T2 t2;
    bool ist1;
    bool isUninitialized = false;
public:
    Sum() {
        isUninitialized = true;
    }
    Sum(T1 it1) :
        t1(it1), ist1(true), isUninitialized(false) {} 
    Sum(T2 it2) :
        t2(it2), ist1(false), isUninitialized(false) {}
    auto isT1() -> bool {
        return ist1 && !isUninitialized;
    }
    auto isT2() -> bool {
        return !ist1 && !isUninitialized;
    }
    auto getT1() -> Expected<T1> {
        if (!ist1 || isUninitialized) return ExpectedErr<T1>();

        return t1;
    }
    auto getT2() -> Expected<T2> {
        if (ist1 || isUninitialized) return ExpectedErr<T2>();

        return t2;
    }
    auto isInitalized() -> bool { return !isUninitialized; }
    operator Expected<T1>() {
        return getT1();
    }
    operator Expected<T2>() {
        return getT2();
    }

};
