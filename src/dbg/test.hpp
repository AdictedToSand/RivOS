#pragma once
#include <gen/vec.hpp>

struct Test {
    struct Result {
        bool passed;
        const char* o_errmsg;

        Result(bool ipassed, const char* io_errmsg) : 
            passed(ipassed), o_errmsg(io_errmsg) {}
    };
    const char* tstname;
    Result (*fn)();

    Test(const char* itstname, Result (*ifn)()) :
        tstname(itstname), fn(ifn) {
    }
};
using TestResult = Test::Result;
extern Vector<Test> tests;
#define TEST(body, fname, tstname) auto fname() -> TestResult { \
    body\
}\
static bool __unused_test_##fname = (tests.pushBack(Test(tstname, fname)), true);
#define PASS() return TestResult(true, "")  
#define FAIL(errmsg) return TestResult(false, errmsg)

auto testTests() -> void;
