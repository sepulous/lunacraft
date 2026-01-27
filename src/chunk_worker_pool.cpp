
#include "chunk_worker_pool.h"

ChunkWorkerPool::ChunkWorkerPool()
{
    size_t worker_count = std::thread::hardware_concurrency() - 1;
    if (worker_count < 1)
        worker_count = 1;

    for (size_t i = 0; i < worker_count; ++i)
        _workers.emplace_back(&ChunkWorkerPool::WorkerLoop, this);
}

ChunkWorkerPool::~ChunkWorkerPool()
{
    {
        std::lock_guard<std::mutex> lock(_jobs_mutex);
        stop = true;
    }

    _jobs_cv.notify_all();

    for (auto &worker : _workers)
        worker.join();
}

void ChunkWorkerPool::SubmitJob(std::function<void()> job)
{
    {
        std::lock_guard<std::mutex> lock(_jobs_mutex);
        _jobs.push(std::move(job));
    }
    _jobs_cv.notify_one();
}

void ChunkWorkerPool::WorkerLoop()
{
    while (true)
    {
        std::function<void()> job;

        {
            std::unique_lock<std::mutex> lock(_jobs_mutex);
            _jobs_cv.wait(lock, [this] {
                return stop || !_jobs.empty();
            });

            if (stop && _jobs.empty())
                return;

            job = std::move(_jobs.front());
            _jobs.pop();
        }

        job();
    }
}
