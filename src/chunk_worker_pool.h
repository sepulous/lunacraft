#pragma once

#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

class ChunkWorkerPool
{
public:
    ChunkWorkerPool();
    ~ChunkWorkerPool();

    void SubmitJob(std::function<void()> job);

private:
    void WorkerLoop();

    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _jobs;

    std::mutex _jobs_mutex;
    std::condition_variable _jobs_cv;
    bool stop = false;
};
