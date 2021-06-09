#include "future_promise.h"
#include <iostream>
#include <thread>

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
    b  = compute(b);
    auto finish = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cerr << "Sequenced: " << elapsed.count() << std::endl;
    std::cerr << "Result: " << a << ", " << b << std::endl;
}

int main() {
    int a = 5, b = 6;
    small_test_promise_future(a, b);
    small_test_sequenced(a, b);
}
