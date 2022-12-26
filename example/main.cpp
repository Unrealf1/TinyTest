#include <sstream>
#include <string>
#include <exception>

#include <tiny_test.hpp>

using testing::make_test;
using testing::make_timed_test;
using testing::PrettyTest;
using testing::SimpleTest;
using testing::TestGroup;

using namespace std::chrono_literals;


TestGroup all_tests[] = {
    // Test Group is constructed from name and sequence of tests
    TestGroup("test group 1", // name doesn't have to be unique
        // SimpleTest is simple: it's made out of name and
        // a predicate with no arguments. If it returns `true`,
        // then test was successful.
        make_test<testing::SimpleTest>("math works", [] { // this name doesn't have to be unique as well
            return (2 + 2) == 4;
        }),

        // If any exception is thrown out of the functor, it is caught and the test is
        // considered failed. This applies to all test types
        make_test<SimpleTest>("exception", [] {
            throw std::runtime_error("this is expected");
            return true; // test will still fail due to an uncought exception
        })
    ),

    TestGroup("string tests", 
        // PrettyTest is a bit more complex: instead of a
        // predicate without arguments it expects a functor that
        // accepts PrettyTest& and returns nothing. Then functor
        // may call .check(bool) method on it's argument, which 
        // acts similar to ASSERT in other test frameworks. If one
        // or more .check() calls recieve false as an argument,
        // the test is failed and all failed checks will print
        // debug info on stdout (if source_location is supported
        // by your compiler)
        make_test<PrettyTest>("push back and length", [](auto& test) {
            std::string str;
            const size_t repeats = 1000;
            for (size_t i = 0; i < repeats; ++i) {
                test.check(i == str.length());
                str.push_back('a');
            }

            // this effectively means "test is failed if `str.at(...)` did _not_ throw an exception"
            try {
                str.at(repeats);
                test.fail(); // equivalent to .check(false)
            } catch(...) { }
        }),

        make_test<PrettyTest>("back & front", [](auto& test){
            const size_t len = 100;
            std::string str(len, 'q');
            char& first = str.front();
            char& last = str.back();
            first = 'a';
            last = 'b';
            // you can freely use any compound expressions
            test.check(str[0] == 'a' && str[len - 1] == 'b' && (&last - &first == len - 1));
        }),

        make_test<PrettyTest>("empty & clear", [](auto& test){
            std::string str;
            test.check(str.empty());
            str = std::string(12, 's');
            test.check(!str.empty());
            str.clear();
            test.check(str.empty());
            str.clear();
        }),

        make_test<PrettyTest>("several writes", [](auto& test) {
            std::string lang = "c++";
            std::string middle = " is the";
            std::string status = "best";

            std::stringstream stream{};
            stream << lang << middle << ' ' << status << "!";
            std::string res = stream.str();
            // equals(a, b) is equivalent to .check(a == b), but it can
            // automatically output error message on fail to ease debugging
            test.equals(res, "c++ is the best!");
            // this will fail and print why
            test.equals(res, "c++ is the worst!");
        }),

        // timed tests are made out of other tests (PrettyTest is this case) and
        // they time execution of the given test using std::clock
        make_timed_test<PrettyTest>("raw push_back perfomance", [](auto& test){
            const size_t repeats = 1'000;
            
            std::string string;
            for (size_t i = 0; i < repeats; ++i) {
                string.push_back('c');
            }
        }),

        // tou can (optionally) give test a max allowed execution time(in microseconds). 
        // If execution takes longer than given time, test will fail. However, 
        // test will run to it's end, job cancellation is not supported
        make_timed_test<PrettyTest>(1us, "reserved push_back perfomance", [](auto& test){
            const size_t repeats = 1'000;
            
            std::string string;
            string.reserve(repeats);
            for (size_t i = 0; i < repeats; ++i) {
                string.push_back('c');
            }
        })
    ),
    TestGroup("third group", 
        make_test<PrettyTest>("float equals", [](auto& test){
            // .float equals(a, b, delta) is equivalent to .check(std::abs(a - b) < delta)
            test.float_equals(1.0, 1.1, 0.11);       
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

