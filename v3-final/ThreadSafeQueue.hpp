#ifndef _THREADSAFE_QUEUE_HPP_
#define _THREADSAFE_QUEUE_HPP_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    ThreadSafeQueue() = default;

    void push(const T& value) {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(value);
        cv.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return !queue.empty(); });
        T value = queue.front();
        queue.pop();
        return value;
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(mtx);
        return queue.empty();
    }
};

#endif
