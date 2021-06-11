#include "is_valid.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <type_traits>

using namespace isValidImpl;

template <typename T>
auto print_integral(std::ostream &os, const T &value)
    -> std::enable_if_t<std::is_integral_v<T>, void> {
    os << value;
}

int main() {
    std::stringstream s;
    auto print_integral_validator =
        isValid([&](const auto &v) -> decltype(print_integral(s, v)) {});

    static_assert(print_integral_validator(1));
    static_assert(!print_integral_validator("kek"));
}
