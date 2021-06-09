#pragma once

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>

struct FuturePromiseException : std::runtime_error {
    using runtime_error::runtime_error;
};

template <class T> struct SharedState : std::enable_shared_from_this<SharedState<T>> {
    T value_;
    std::exception_ptr exception_;
    bool ready_ = false;
    std::mutex mtx_;
    std::condition_variable cv_;
};

template <class T> struct Future {
    Future(std::shared_ptr<SharedState<T>> state) : state_(std::move(state)) {}

    void wait() const {
        if (!state_) {
            throw FuturePromiseException("No state");
        }
        std::unique_lock lock(state_->mtx_);
        while (!state_->ready_)
            state_->cv_.wait(lock);
    }

    bool valid() const {;
        return state_ != nullptr;
    }

    bool ready() const {
        if (!valid()) return false;
        std::lock_guard lock(state_->mtx_);
        return state_->ready;
    }

    T get() {
        wait(); 
        auto sp = std::move(state_);
        if (sp->exception_) {
            std::rethrow_exception(sp->exception_);
        }
        return std::move(sp->value_);
    }

  private:
    std::shared_ptr<SharedState<T>> state_;
};

template <class T> struct Promise {
    // dummy
    Promise() {
        state_ = std::make_shared<SharedState<T>>();
    }
    void setValue(T value) {
        if (!state_) {
            throw FuturePromiseException("No state");
        }
        std::lock_guard lock(state_->mtx_);
        if (state_->ready_) {
            throw FuturePromiseException("Already satisfied");
        }

        state_->value_ = value;
        state_->ready_ = true;
        state_->cv_.notify_all();
    }
    void setException(std::exception_ptr ptr) {
        if (!state_) {
            throw FuturePromiseException("No state");
        }
        std::lock_guard lock(state_.mtx_);
        if (state_->ready_) {
            throw FuturePromiseException("Already satisfied");
        }
        state_->exception_ = ptr;
        state_->ready = true;
        state_->cv_.notify_all();
    }

    Future<T> getFuture() {
        if (!state_) {
            throw FuturePromiseException("No state");
        }
        if (already_retrieved_) {
            throw FuturePromiseException("Future has already retrieved");
        }
        already_retrieved_ = true;
        return Future(state_);
    }

  private:
    std::shared_ptr<SharedState<T>> state_;
    bool already_retrieved_ = false;
};
