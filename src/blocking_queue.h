#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <stop_token>

template <typename T>
class BlockingQueue
{
    private:
        std::queue<T> _queue;
        std::mutex _mutex;
        std::condition_variable_any _cv;

    public:
        void Push(T value)
        {
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _queue.push(std::move(value));
            }
            _cv.notify_one();
        }

        bool Pop(T &out, std::stop_token stoken)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, stoken, [&] { return !_queue.empty(); });

            if (stoken.stop_requested())
                return false;

            out = std::move(_queue.front());
            _queue.pop();
            return true;
        }

        bool TryPop(T &out)
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (_queue.empty())
                return false;

            out = std::move(_queue.front());
            _queue.pop();
            return true;
        }
};

#endif
