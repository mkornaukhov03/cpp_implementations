#pragma once

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>

struct FuturePromiseException : std::runtime_error {
    using runtime_error::runtime_error;
};

template <class T> struct SharedState {
    T value_;
    std::exception_ptr exception_;
    bool ready_ = false;
    std::mutex mtx_;
    std::condition_variable cv_;
};

template <class T> class Future {
    Future(const std::shared_ptr<SharedState<T>> &state) : state_(state) {}

    void wait() const {
        if (!state_) {
            throw FuturePromiseException("No state");
        }
        std::scoped_lock lock(state_->mtx_);
        while (!state_->ready_)
            state_->cv_.wait(lock);
    }

    bool valid() const {
        std::lock_guard lock(state_->mtx_);
        return state_ != nullptr;
    }

    bool ready() const {
        if (!state_)
            return false;
        std::lock_guard lock(state_->mtx_);
        return state_->ready;
    }

    T get() {
        wait(); 
        auto sp = std::move(state_);
        if (sp->exception) {
            std::rethrow_exception(sp->exception);
        }
        return std::move(sp->value);
    }

  private:
    std::shared_ptr<SharedState<T>> state_;
};

template <class T> class Promise {
    // dummy
    void setValue(T value) {
        if (!state_) {
            throw FuturePromiseException("No state");
        }
        std::lock_guard lock(state_.mtx_);
        if (state_->ready_) {
            throw FuturePromiseException("Already satisfied");
        }

        state_->value = value;
        state_->ready = true;
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
