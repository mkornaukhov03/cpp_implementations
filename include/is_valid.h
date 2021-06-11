#pragma once
// Simplified implementation of Boosh::hana::is_valid

#include <utility>

namespace isValidImpl {
template <typename F, typename... Args,
          typename = decltype(std::declval<F>()(std::declval<Args &&>()...))>
constexpr auto validImpl(int) {
    return true;
}

template <typename F, typename... Args> constexpr auto validImpl(...) {
    return false;
}

template <typename F> struct valid_fun {
    template <typename... Args> constexpr bool operator()(Args &&...) {
        return validImpl<F, Args &&...>(42);
    }
};

template <typename F> auto isValid(const F &) { return valid_fun<F>{}; }
} // namespace isValidImpl
