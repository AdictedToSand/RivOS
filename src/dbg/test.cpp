#include <dbg/test.hpp>

#include <terminal/terminal.hpp>

Vector<Test> tests;

auto testTests() -> void {
    u32 passedTests = 0;
    u32 failedTests = 0;
    for (auto testToCheck : tests) {
        const auto fn = testToCheck.fn;
        const TestResult rslt = fn();

        if (!rslt.passed) {
            Serial::logf("Test %s failed with %s", testToCheck.tstname, rslt.o_errmsg);
            Terminal::printfColor("Test failed (test %s: %s)\n", (u32) Terminal::VgaColor::Red, testToCheck.tstname, rslt.o_errmsg);
            failedTests++;
            continue;
        }
        Serial::logf("Test passed: %s", testToCheck.tstname);
        passedTests++;
    }

    if (failedTests == 0)
        Terminal::printfColor("All tests passed (total: %u)", (u32) Terminal::VgaColor::Green, passedTests);
    else if (passedTests > failedTests) 
        Terminal::printfColor("Most tests passed (passed: %u, failed: %u)", (u32) Terminal::VgaColor::Blue, passedTests, failedTests);
    else
        Terminal::printfColor("Most tests failed! (failed: %u, passed: %u)", (u32) Terminal::VgaColor::Red, failedTests, passedTests);
}

TEST({
    PASS();
}, testTestsSingle, "TestTestsTest LMAO");
TEST({
    FAIL("Something failed!!");
}, testFailure, "TestFailure");
TEST({
    PASS();
}, testPass, "TestPass")
