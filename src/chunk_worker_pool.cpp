
#include "chunk_worker_pool.h"

//
// ChunkWorkerPool
//

ChunkWorkerPool::ChunkWorkerPool()
{
    size_t worker_count = std::thread::hardware_concurrency() - 1;
    if (worker_count < 1)
        worker_count = 1;

    for (size_t i = 0; i < worker_count; ++i)
        workers_.emplace_back(&ChunkWorkerPool::WorkerLoop, this);
}

ChunkWorkerPool::~ChunkWorkerPool()
{
    {
        std::lock_guard<std::mutex> lock(jobs_mutex_);
        stop_ = true;
    }

    jobs_cv_.notify_all();

    for (auto &worker : workers_)
        worker.join();
}

void ChunkWorkerPool::SubmitJob(WorkerJob job)
{
    {
        std::lock_guard<std::mutex> lock(jobs_mutex_);
        jobs_.push(std::move(job));
    }
    jobs_cv_.notify_one();
}

void ChunkWorkerPool::WorkerLoop()
{
    while (true)
    {
        WorkerJob job;

        {
            std::unique_lock<std::mutex> lock(jobs_mutex_);
            jobs_cv_.wait(lock, [this] {
                return stop_ || !jobs_.empty();
            });

            if (stop_ && jobs_.empty())
                return;

            job = std::move(jobs_.front());
            jobs_.pop();
        }

        for (auto task : job.tasks)
            (job.chunk->*task)();
    }
}
