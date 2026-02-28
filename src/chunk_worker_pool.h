#pragma once

#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "chunk.h"

struct ChunkJob
{
    Chunk *chunk;
    std::vector<bool (Chunk::*)()> tasks;
    size_t current_task = 0;

    bool ExecuteNextTask();
    bool IsDone();
};

class ChunkWorkerPool
{
public:
    ChunkWorkerPool();
    ~ChunkWorkerPool();

    void SubmitJob(ChunkJob job);

private:
    void WorkerLoop();

    std::vector<std::thread> _workers;
    std::queue<ChunkJob> _jobs;

    std::mutex _jobs_mutex;
    std::condition_variable _jobs_cv;
    bool stop = false;
};
