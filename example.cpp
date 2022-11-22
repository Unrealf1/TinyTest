// To compile this example, simply call your preferred compiler
// with this file as an argument: `g++ example.cpp`, `clang++ example.cpp`
// Add -DUSE_SOURCE_LOCATION for detailed debug output if your compiler
// supports std::source_location
#include <sstream>
#include <string>
#include <exception>

#include "tiny_test.hpp"

using testing::make_pretty_test;
using testing::make_simple_test;
using testing::TestGroup;


TestGroup all_tests[] = {
    // Test Group is constructed from name and sequence of tests
    TestGroup("test group 1", // name doesn't have to be unique
        // simple test is simple: it's made out of name and
        // a predicate with no arguments. If it returns `true`,
        // then test was successful.
        make_simple_test("math works", [] { // this name doesn't have to be unique as well
            return (2 + 2) == 4;
        }),

        // If any exception is thrown out of the functor, it is caught and the test is
        // considered failed. This applies to all test types
        make_simple_test("exception", [] {
            throw std::runtime_error("this is expected");
            return true;
        })
    ),

    TestGroup("string tests", 
        // pretty tests are a bit more complex: instead of a
        // predicate without arguments they expect a functor that
        // accepts PrettyTest& and returns nothing. Then functor
        // may call .check(bool) method on it's argument, which 
        // acts similar to ASSERT in other test frameworks. If one
        // or more .check() calls recieve false as an argument,
        // the test is failed and all failed checks will print
        // debug info on stdout (if source_location is supported
        // by your compiler)
        make_pretty_test("push back and length", [](auto& test) {
            std::string str;
            const size_t repeats = 1000;
            for (size_t i = 0; i < repeats; ++i) {
                test.check(i == str.length());
                str.push_back('a');
            }

            try {
                str.at(repeats);
                test.fail(); // equivalent to .check(false)
            } catch(...) { }
        }),

        make_pretty_test("back & front", [](auto& test){
            const size_t len = 100;
            std::string str(len, 'q');
            char& first = str.front();
            char& last = str.back();
            first = 'a';
            last = 'b';
            test.check(str[0] == 'a' && str[len - 1] == 'b' && (&last - &first == len - 1));
        }),

        make_pretty_test("empty & clear", [](auto& test){
            std::string str;
            test.check(str.empty());
            str = std::string(12, 's');
            test.check(!str.empty());
            str.clear();
            test.check(str.empty());
            str.clear();
        }),

        make_pretty_test("several writes", [](auto& test) {
            std::string lang = "c++";
            std::string middle = " is the";
            std::string status = "best";

            std::stringstream stream{};
            stream << lang << middle << ' ' << status << "!";
            std::string res = stream.str();
            test.check(res == "c++ is the best!");
        })
    )
};

int main() {
    bool success = true;
    for (auto& group : all_tests) {
        success &= group.run();
    }
    return success ? 0 : 1;
}

