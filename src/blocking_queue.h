#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class BlockingQueue
{
    private:
        std::queue<T> _queue;
        std::mutex _mutex;
        std::condition_variable _cv;
        bool _stopped = false;

    public:
        void Push(T value)
        {
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _queue.push(std::move(value));
            }
            _cv.notify_one();
        }

        T Pop()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, [&] { return !_queue.empty() || _stopped; });

            if (_stopped)
                return T{};

            T value = std::move(_queue.front());
            _queue.pop();
            return value;
        }

        bool TryPop(T& out)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_queue.empty())
                return false;

            out = std::move(_queue.front());
            _queue.pop();
            return true;
        }

        void Stop()
        {
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _stopped = true;
            }
            _cv.notify_all();
        }

        bool IsStopped()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return _stopped;
        }
};

#endif
