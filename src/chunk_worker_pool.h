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

    std::vector<std::thread> _workers;
    std::queue<WorkerJob> _jobs;

    std::mutex _jobs_mutex;
    std::condition_variable _jobs_cv;
    bool stop = false;
};
