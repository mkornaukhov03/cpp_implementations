#include "future_promise_simplified.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <type_traits>

namespace future_promise_test {
using namespace FuturePromise;
int compute(int x) {
    constexpr std::size_t STEPS = 500'000'000;
    constexpr int MOD = 10'000;
    for (std::size_t i = 0; i < STEPS; ++i) {
        x += (i + 3 * i) % MOD * (x + 2) % MOD;
    }
    return x;
}

void small_test_promise_future(int a, int b) {
    auto start = std::chrono::steady_clock::now();

    Promise<int> pa, pb;
    Future<int> fa = pa.getFuture(), fb = pb.getFuture();

    std::thread t1([&]() { pa.setValue(compute(a)); });
    std::thread t2([&]() { pb.setValue(compute(b)); });
    t1.join();
    t2.join();
    auto finish = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cerr << "With future-promise: " << elapsed.count() << std::endl;
    std::cerr << "Result: " << fa.get() << ", " << fb.get() << std::endl;
}

void small_test_sequenced(int a, int b) {
    auto start = std::chrono::steady_clock::now();

    a = compute(a);
    b = compute(b);
    auto finish = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cerr << "Sequenced: " << elapsed.count() << std::endl;
    std::cerr << "Result: " << a << ", " << b << std::endl;
}
} // namespace future_promise_test

int main() {
    using namespace future_promise_test;
    small_test_promise_future(4, 5);
    small_test_sequenced(4, 5);
}
