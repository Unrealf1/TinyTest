#pragma once

#include <exception>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <limits>
#include <chrono>
#include <string_view>

#ifndef TINY_TEST__NO_SOURCE_LOCATION
#include <source_location>
#endif


namespace testing {
    template <typename T>
    constexpr auto type_name() {
        std::string_view name, prefix, suffix;
#ifdef __clang__
        name = __PRETTY_FUNCTION__;
        prefix = "auto type_name() [T = ";
        suffix = "]";
#elif defined(__GNUC__)
        name = __PRETTY_FUNCTION__;
        prefix = "constexpr auto type_name() [with T = ";
        suffix = "]";
#elif defined(_MSC_VER)
        name = __FUNCSIG__;
        prefix = "auto __cdecl type_name<";
        suffix = ">(void)";
#endif
        name.remove_prefix(prefix.size());
        name.remove_suffix(suffix.size());
        return name;
    }

    using namespace std::chrono_literals;

    template<typename T>
    concept Printable = requires(T item, std::ostream ostr) {
        { ostr << item };
    };

    // Base Test class. All other tests should inherit from it
    // and override `doTest` method
    class Test {
    public:
        Test(std::string name): name_(std::move(name)) {}

        virtual ~Test() = default;

        bool operator()() {
            std::cout << "test \"" << name_ << "\"\n" << std::flush;
            bool res = false;
            try {
                res = doTest();
            } catch(std::exception& exception) {
                std::cout << "caught exception: " << exception.what() << '\n';
            } catch (...) {
                std::cout << "caught unknown exception\n";
            }
            std::cout << "[\x1B[" << (res ? "32mOK" : "31mFAIL") << "\033[0m]\n";
            return res;
        }

    protected:
        std::string name_;

        virtual bool doTest() = 0;
    };

    // Wrapper around any Functor that takes nothing and returns bool
    template<typename Functor>
    class SimpleTest: public Test {
    public:
        SimpleTest(std::string name, Functor f)
        : Test(std::move(name))
        , f_(std::move(f)) {}

        bool doTest() override {
            return f_();
        }

    private:
        Functor f_;
    };

    // Main test class, provides lots of helper methods and automatic output
    // Functor is called with PrettyTest instance as an argument
    template<typename Functor>
    class PrettyTest: public Test {
    public:
        PrettyTest(const PrettyTest&) = delete;
        PrettyTest(PrettyTest&&) = delete;

        PrettyTest(std::string name, Functor f)
        : Test(std::move(name))
        , f_(std::move(f)) {}

        bool doTest() override {
            f_(*this);
            return result_;
        }

#ifndef TINY_TEST__NO_SOURCE_LOCATION
        bool check(bool condition, const std::source_location location = std::source_location::current()) {
            result_ &= condition;
            if (condition == false) {
                std::cout
                    << "condition at " << location.file_name()
                    << ", line " << location.line()
                    << ':' << location.column()
                    << " evaluated to false\n";
            }
            return condition;
        }

        template<typename First, typename Second>
        requires Printable<First> && Printable<Second>
        bool equals(
                First& first,
                Second&& second,
                const std::source_location location = std::source_location::current()) {
            const bool result = check(first == second, location);
            if (!result) {
                std::cout << first << " (" << type_name<First>() << ") != "
                    << second << " (" <<  type_name<Second>() << ")\n";
            }
            return result;
        }

        bool equals(
                auto&& first,
                auto&& second,
                const std::source_location location = std::source_location::current()) {
            return check(first == second, location);
        }

        template<typename Float>
        requires std::floating_point<Float>
        bool float_equals(Float x, Float y, Float error, const std::source_location location = std::source_location::current()) {
            const bool result = check(std::abs(x - y) < error, location);
            if (!result) {
                std::cout << std::setprecision(4) << x << " != " << y << " with epsilon " << error << '\n';
            }
            return result;
        }

        bool fail(const std::source_location location = std::source_location::current()) {
            return check(false, location);
        }
#else
        bool check(bool condition) {
            result_ &= condition;
            return condition;
        }

        bool fail() {
            return check(false);
        }

        bool equals(auto&& first, auto&& second) {
            return check(first == second);
        }

        template<typename Float>
        bool float_equals(Float x, Float y, Float error) {
            const bool result = check(std::abs(x - y) < error);
            if (!result) {
                std::cout << std::setprecision(4) << first << " != " second << " with epsilon " << error << '\n';
            }
            return result;
        }
#endif

    private:
        Functor f_;
        bool result_ = true;
    };

    // Helper function for unique_ptr creation of SimpleTest and PrettyTest
    template<template<typename> typename ActualTest, typename Functor>
    std::unique_ptr<ActualTest<Functor>> make_test(
            std::string name,
            Functor f) {
        return std::make_unique<ActualTest<Functor>>(std::move(name), std::move(f));
    }

    // Wrapper around another test. Will time execution and check that it
    // did not exceed a given time limit
    template<template<typename> typename ActualTest, typename Functor>
    struct TimedTest : ActualTest<Functor> {
        using Parent = ActualTest<Functor>;
        using Parent::Parent;

        template<typename... Args>
        TimedTest(double milliseconds, Args&&... args)
            : Parent(std::forward<Args>(args)...)
            , max_runtime_(milliseconds) {}

        bool doTest() override {
            auto start = std::clock();
            auto result = Parent::doTest();
            auto finish = std::clock();
            double execution_ms = double(finish - start) * 1000.0 / CLOCKS_PER_SEC;
            std::cout << "finished in " << std::setprecision(2) << execution_ms << "ms\n";
            if (max_runtime_ < execution_ms) {
                std::cout << "SLOWER than given limit: " << std::setprecision(2) << max_runtime_ << "ms\n";
                return false;
            }
            return result;
        }

    private:
        double max_runtime_ = std::numeric_limits<double>::infinity();
    };

    // Helper function for unique_ptr creation of TimedTest from Simple and Pretty tests
    template<template<typename> typename ActualTest, typename Functor>
    auto make_timed_test(
        std::string name,
        Functor f
    ) {
        return std::make_unique<TimedTest<ActualTest, Functor>>(std::move(name), std::move(f));
    }

    template<template<typename> typename ActualTest, typename Functor>
    auto make_timed_test(
        std::chrono::microseconds time_limit,
        std::string name,
        Functor f
    ) {
        const double to_milliseconds = 1e-3;
        return std::make_unique<TimedTest<ActualTest, Functor>>(
                double(time_limit.count()) * to_milliseconds, std::move(name), std::move(f)
        );
    }

    // Owning container for a group of tests
    class TestGroup {
    public:
        TestGroup(const TestGroup&) = delete;
        TestGroup(TestGroup&&) = default;
        TestGroup(std::string name): name_(std::move(name)) {}

        template<typename FirstTest, typename... OtherTests>
        TestGroup(std::string name, FirstTest first, OtherTests... other)
        : TestGroup(std::move(name)
        , std::move(other)...)
        {
            add(std::move(first));
        }

        void add(std::unique_ptr<Test> test) {
            tests_.push_back(std::move(test));
        }

        bool run() {
            std::cout << "Running group \"" << name_ << '\"' << std::endl;
            size_t errors = 0;
            for (auto& test : tests_) {
                bool result = (*test)();
                if (!result) {
                    ++errors;
                }
            }

            bool result = (errors == 0);
            if (!result) {
                std::cout << "Group failed!\n";
                std::cout << "Failed " << errors << '/' << tests_.size() << " tests\n";
            }
            return result;
        }

    private:
        std::string name_;
        std::vector<std::unique_ptr<Test>> tests_;
    };
}

