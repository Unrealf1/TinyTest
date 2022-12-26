// this file is needed for CMake 'try_compile' feature.
// If you are using `tiny_test.hpp` without included CMakeLists.txt
// then you do not need this file
#include "tiny_test.hpp"

int main() {
    using testing::make_test;
    using testing::PrettyTest;

    // test for source_location availability
    auto test = make_test<PrettyTest>("", [](auto&){});
    test->equals(1, 1);
}
