#pragma once

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>

namespace FuturePromise {
template <class T> struct Promise;

struct FuturePromiseException : std::runtime_error {
    using runtime_error::runtime_error;
};

template <class T>
struct SharedState : std::enable_shared_from_this<SharedState<T>> {
    SharedState() : value_() {}
    std::optional<T> value_;
    std::exception_ptr exception_;
    bool ready_ = false;
    std::mutex mtx_;
    std::condition_variable cv_;
};

template <class T> struct Future {

    Future(const Future &) = delete;
    Future &operator=(const Future &) = delete;

    void wait() const {
        std::unique_lock lock(state_->mtx_);
        while (!state_->ready_)
            state_->cv_.wait(lock);
    }

    bool valid() const { return state_ != nullptr; }

    bool ready() const {
        if (!valid())
            return false;
        std::lock_guard lock(state_->mtx_);
        return state_->ready;
    }

    T get() {
        wait();
        auto sp = std::move(state_);
        if (sp->exception_) {
            std::rethrow_exception(sp->exception_);
        }
        return std::move(sp->value_).value();
    }

    friend struct Promise<T>;

  private:
    Future(std::shared_ptr<SharedState<T>> state) : state_(std::move(state)) {}
    std::shared_ptr<SharedState<T>> state_;
};

template <class T> struct Promise {
    Promise() { state_ = std::make_shared<SharedState<T>>(); }
    Promise &operator=(Promise &&oth) { state_ = std::move(oth.state_); }
    void setValue(T value) {
        std::lock_guard lock(state_->mtx_);
        state_->value_ = value;
        state_->ready_ = true;
        state_->cv_.notify_all();
    }
    void setException(std::exception_ptr ptr) {
        std::lock_guard lock(state_.mtx_);
        state_->exception_ = ptr;
        state_->ready = true;
        state_->cv_.notify_all();
    }

    Future<T> getFuture() {
        return Future(state_);
    }

    ~Promise() {}

  private:
    std::shared_ptr<SharedState<T>> state_;
};
} // namespace FuturePromise
