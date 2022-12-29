// this file is needed for CMake 'try_compile' feature.
// If you are using `tiny_test.hpp` without included CMakeLists.txt
// then you do not need this file
#include <source_location>

int main() {
    auto location = std::source_location::current();
    auto name = location.file_name();
}

