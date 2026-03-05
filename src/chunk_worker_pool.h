#pragma once

#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "chunk.h"

struct WorkerJob
{
    Chunk *chunk;
    std::vector<void (Chunk::*)()> tasks;
};

class ChunkWorkerPool
{
public:
    ChunkWorkerPool();
    ~ChunkWorkerPool();

    void SubmitJob(WorkerJob job);

private:
    void WorkerLoop();

    std::vector<std::thread> workers_;
    std::queue<WorkerJob> jobs_;

    std::mutex jobs_mutex_;
    std::condition_variable jobs_cv_;
    bool stop_ = false;
};
