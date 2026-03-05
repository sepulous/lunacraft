
#include <algorithm>
#include <filesystem>
#include <string>
#include <fstream>
#include <thread>
#include <stop_token>

#include <stb_image/stb_image.h>

#include "chunk_manager.h"
#include "chunk_generation.h"
#include "helpers.h"
#include "storage.h"
#include "options.h"
#include "moon.h"

//
// Chunk tasks
//

void (Chunk::*ChunkTask::LOAD_BLOCKS)() = &Chunk::LoadBlocks;
void (Chunk::*ChunkTask::BUILD_LIGHTMAP_INTERNAL)() = &Chunk::BuildLightmapInternal;
void (Chunk::*ChunkTask::BUILD_LIGHTMAP_EXTERNAL)() = &Chunk::BuildLightmapExternal;
void (Chunk::*ChunkTask::UPDATE_VERTEX_LIGHTING)() = &Chunk::UpdateVertexLighting;
void (Chunk::*ChunkTask::BUILD_VERTICES)() = &Chunk::BuildVertices;
void (Chunk::*ChunkTask::MARK_AS_CLEAN)() = &Chunk::MarkAsClean;

//
// ChunkManager
//

ChunkManager::~ChunkManager()
{
    glDeleteTextures(1, &texture_atlas_);

    // Stop worker pool
    delete worker_pool_;

    // Free chunks
    for (auto [chunk_id, chunk] : chunks_)
        delete chunk;

    // Free block memory
    for (auto &memory : block_memory_)
        free(memory.blocks);
}

void ChunkManager::Init(int moon_id, MoonSettings moon_settings)
{
    // Load texture atlas
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    std::filesystem::path atlas_path = Storage::IMAGES / "texture_atlas.png";
    unsigned char *texture_atlas_data = stbi_load(reinterpret_cast<const char *>(atlas_path.u8string().c_str()), &width, &height, &nrChannels, 0);

    glGenTextures(1, &texture_atlas_);
    glBindTexture(GL_TEXTURE_2D, texture_atlas_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_atlas_data);

    stbi_image_free(texture_atlas_data);

    // Create chunks folder
    std::filesystem::path moon_dir = Storage::MOONS / (std::string("moon") + std::to_string(moon_id));
    std::filesystem::path chunk_dir = moon_dir / "chunks";
    if (!std::filesystem::exists(chunk_dir))
        std::filesystem::create_directory(chunk_dir);

    // Reserve for maximum number of possible memory blocks (including border chunks)
    block_memory_.reserve((2*MAX_RENDER_DISTANCE + 3) * (2*MAX_RENDER_DISTANCE + 3));

    // Start worker pool
    worker_pool_ = new ChunkWorkerPool{};
}

void ChunkManager::HandleChunkJobs()
{
    int jobs_to_handle = job_queue_.size(); // Jobs can be requeued, so we should only get this once at the beginning
    for (int i = 0; i < jobs_to_handle; i++)
    {
        if (job_queue_.front().chunk->IsDirty())
            continue;

        auto job = std::move(job_queue_.front());
        job_queue_.pop();

        if (job.requires_neighbors)
        {
            auto neighbors = GetNeighbors(job.chunk->GetCoords());
            bool neighbors_ready = std::none_of(neighbors.begin(), neighbors.end(), [](auto neighbor) {
                return neighbor->GetState() < ChunkState::INTERNAL_DONE;
            });

            if (neighbors_ready) // All tasks can be done
            {
                job.chunk->MarkAsDirty();
                worker_pool_->SubmitJob({job.chunk, job.tasks});
            }
            else // External tasks cannot be done, but there may be internal tasks that can
            {
                auto it = std::partition_point(job.tasks.begin(), job.tasks.end(), [](auto task) {
                    return task == ChunkTask::LOAD_BLOCKS || task == ChunkTask::BUILD_LIGHTMAP_INTERNAL;
                });

                if (it == job.tasks.begin()) // All tasks are external, so just requeue the job
                {
                    job_queue_.push(std::move(job));
                }
                else
                {
                    job.chunk->MarkAsDirty();

                    // Submit internal tasks
                    std::vector<void (Chunk::*)()> internal_tasks{job.tasks.begin(), it};
                    internal_tasks.push_back(ChunkTask::MARK_AS_CLEAN);
                    worker_pool_->SubmitJob({job.chunk, internal_tasks});

                    // Requeue remaining external tasks
                    job.tasks = std::vector<void (Chunk::*)()>{it, job.tasks.end()};
                    job_queue_.push(std::move(job));
                }
            }
        }
        else // Purely internal job; just submit everything
        {
            job.chunk->MarkAsDirty();
            worker_pool_->SubmitJob({job.chunk, job.tasks});
        }
    }
}

void ChunkManager::UploadReadyChunks()
{
    for (auto it = chunks_.begin(); it != chunks_.end(); ++it)
    {
        Chunk *chunk = it->second;
        if (!chunk->IsBorderChunk() && chunk->GetState() == ChunkState::READY_TO_UPLOAD) // Shouldn't waste GPU memory with border chunks. Most are never rendered.
        {
            chunk->UploadVertices();
            loaded_chunk_count_++;
        }
    }
}

void ChunkManager::HandlePlayerModification(glm::ivec3 voxel, BlockID block_placed)
{
    // Get chunk
    auto chunk_coords = VoxelToChunk(voxel);
    auto chunk = chunks_.at(ChunkCoordsToID(chunk_coords));

    // Remove block
    auto local = GlobalToLocalVoxel(voxel);
    chunk->GetBlocks()[GetChunkIndex(local)] = block_placed;

    // Rebuild this chunk
    chunk->PinNeighbors();
    job_queue_.push({
        .chunk = chunk,
        .requires_neighbors = true,
        .tasks = {
            ChunkTask::BUILD_LIGHTMAP_INTERNAL,
            ChunkTask::BUILD_LIGHTMAP_EXTERNAL,
            ChunkTask::BUILD_VERTICES,
            ChunkTask::MARK_AS_CLEAN
        }
    });

    // Rebuild neighbor chunks
    auto neighborchunks_ = GetNeighbors(chunk_coords);
    auto voxel_local = GlobalToLocalVoxel(voxel);
    if (voxel_local.x != 0 && voxel_local.x != CHUNK_SIZE - 1 && voxel_local.z != 0 && voxel_local.z != CHUNK_SIZE - 1) // Not on chunk border
    {
        for (auto &neighbor : neighborchunks_)
        {
            neighbor->PinNeighbors();
            job_queue_.push({
                .chunk = neighbor,
                .requires_neighbors = true,
                .tasks = {
                    ChunkTask::BUILD_LIGHTMAP_INTERNAL,
                    ChunkTask::BUILD_LIGHTMAP_EXTERNAL,
                    ChunkTask::UPDATE_VERTEX_LIGHTING,
                    ChunkTask::MARK_AS_CLEAN
                }
            });
        }
    }
    else // On chunk border
    {
        int indices[] = {2, 0, 2, 0}; // {z, x, z, x}
        int borders[] = {CHUNK_SIZE - 1, CHUNK_SIZE - 1, 0, 0}; // {z_max, x_max, z_min, x_min}
        for (int i = 0; i < 4; i++)
        {
            // This is just a compact way to decide whether the modified voxel shares a border with
            // the ith neighbor, to decide how that neighbor should update its vertices. If a shared
            // border was modified, the neighbor must remesh; otherwise, it only needs to update lighting.
            auto vertex_task = voxel_local[indices[i]] == borders[i] ? ChunkTask::BUILD_VERTICES
                                                                     : ChunkTask::UPDATE_VERTEX_LIGHTING;
            auto neighbor = neighborchunks_[i];
            neighbor->PinNeighbors();
            job_queue_.push({
                .chunk = neighbor,
                .requires_neighbors = true,
                .tasks = {
                    ChunkTask::BUILD_LIGHTMAP_INTERNAL,
                    ChunkTask::BUILD_LIGHTMAP_EXTERNAL,
                    vertex_task,
                    ChunkTask::MARK_AS_CLEAN
                }
            });
        }
    }
}

void ChunkManager::RenderChunks(Plane frustum[6])
{
    glBindTexture(GL_TEXTURE_2D, texture_atlas_);
    glDepthFunc(GL_LESS);

    std::vector<Chunk *> visiblechunks_;
    visiblechunks_.reserve(32);

    // Render opaque blocks
    for (auto it = chunks_.begin(); it != chunks_.end(); ++it)
    {
        Chunk *chunk = it->second;
        if (chunk->HasUploadedVertices() && !chunk->IsBorderChunk())
        {
            glm::ivec3 chunk_coords = chunk->GetCoords();
            float x0 = chunk_coords.x * CHUNK_SIZE;
            float z0 = chunk_coords.z * CHUNK_SIZE;
            if (ChunkInFrustum(frustum, x0, z0))
            {
                chunk->RenderOpaques();
                visiblechunks_.push_back(chunk);
            }
        }
    }

    // Render transparent blocks
    for (Chunk *chunk : visiblechunks_)
    {
        chunk->RenderTransparents();
    }
}

void ChunkManager::UpdateGlobalLighting()
{
    for (auto it = chunks_.begin(); it != chunks_.end(); ++it)
    {
        Chunk *chunk = it->second;
        if (!chunk->IsBorderChunk())
        {
            chunk->PinNeighbors();
            job_queue_.push({
                .chunk = chunk,
                .requires_neighbors = true,
                .tasks = {
                    ChunkTask::UPDATE_VERTEX_LIGHTING,
                    ChunkTask::MARK_AS_CLEAN
                }
            });
        }
    }
}

//
// Create initial chunk patch around player spawn.
//
void ChunkManager::CreateInitialPatch()
{
    // Create all chunks
    auto player_chunk = VoxelToChunk(GetNearestVoxel(Moon::GetCurrentMoon()->GetPlayer()->GetPosition()));
    int render_distance = OptionsManager::GetOptions().render_distance;
    for (int x = player_chunk.x - render_distance - 1; x <= player_chunk.x + render_distance + 1; x++)
    {
        for (int z = player_chunk.z - render_distance - 1; z <= player_chunk.z + render_distance + 1; z++)
        {
            auto chunk_id = ChunkCoordsToID({x, 0, z});
            bool is_border_chunk = x == player_chunk.x - render_distance - 1
                                || x == player_chunk.x + render_distance + 1
                                || z == player_chunk.z - render_distance - 1
                                || z == player_chunk.z + render_distance + 1;

            chunks_.emplace(chunk_id, new Chunk(glm::ivec3{x, 0, z}, is_border_chunk, this)); // This must succeed so the new chunk isn't leaked
        }
    }

    // Chunks expect their neighbors to exist when building, so we defer it
    for (auto it = chunks_.begin(); it != chunks_.end(); ++it)
    {
        auto chunk = it->second;
        if (chunk->IsBorderChunk())
        {
            job_queue_.push({
                .chunk = chunk,
                .requires_neighbors = false,
                .tasks = {
                    ChunkTask::LOAD_BLOCKS,
                    ChunkTask::BUILD_LIGHTMAP_INTERNAL,
                    ChunkTask::MARK_AS_CLEAN
                }
            });
        }
        else
        {
            chunk->PinNeighbors();
            job_queue_.push({
                .chunk = chunk,
                .requires_neighbors = true,
                .tasks = {
                    ChunkTask::LOAD_BLOCKS,
                    ChunkTask::BUILD_LIGHTMAP_INTERNAL,
                    ChunkTask::BUILD_LIGHTMAP_EXTERNAL,
                    ChunkTask::BUILD_VERTICES,
                    ChunkTask::MARK_AS_CLEAN
                }
            });
        }
    }
}

// Get chunk's neighbors in order {front, right, back, left}
std::array<Chunk *, 4> ChunkManager::GetNeighbors(glm::ivec3 chunk_coords)
{
    std::array<Chunk *, 4> neighbors;

    glm::ivec3 neighbor_coords[] = {
        {chunk_coords.x, 0, chunk_coords.z + 1}, // Front
        {chunk_coords.x + 1, 0, chunk_coords.z}, // Right
        {chunk_coords.x, 0, chunk_coords.z - 1}, // Back
        {chunk_coords.x - 1, 0, chunk_coords.z}  // Left
    };

    size_t idx = 0;
    for (auto &neighbor : neighbor_coords)
    {
        auto chunk_id = ChunkCoordsToID(neighbor);
        neighbors[idx] = chunks_.at(chunk_id); // Existence of neighbors should be guaranteed
        idx++;
    }

    return neighbors;
}

void ChunkManager::AdjustChunkPatch()
{
    auto player_chunk = VoxelToChunk(GetNearestVoxel(Moon::GetCurrentMoon()->GetPlayer()->GetPosition()));
    auto render_distance = OptionsManager::GetOptions().render_distance;

    // Remove all chunks outside the patch + border
    for (auto it = chunks_.begin(); it != chunks_.end(); )
    {
        auto &chunk = it->second;

        glm::ivec3 coords = chunk->GetCoords();
        bool marked_for_delete = chunk->IsMarkedForDelete();
        bool outside_border = coords.x < player_chunk.x - render_distance - 1
                          || coords.x > player_chunk.x + render_distance + 1
                          || coords.z < player_chunk.z - render_distance - 1
                          || coords.z > player_chunk.z + render_distance + 1;

        if (marked_for_delete || outside_border)
        {
            chunk->MarkForDelete();
            if (chunk->GetPinCount() < 1)
            {
                // // Write chunk to disk and then take block memory back
                // auto file_path = chunk->GetFilePath();
                // auto blocks = chunk->GetBlocks();
                // auto chunk_id = chunk->GetID();
                // worker_pool_->SubmitJob([this, file_path, blocks, chunk_id]() {
                //     WriteChunkToDisk(file_path, blocks);
                //     ReuseBlockMemory(chunk_id);
                // });

                // Erase chunk
                delete chunk;
                it = chunks_.erase(it);
                loaded_chunk_count_--;
            }
        }
        else
        {
            ++it;
        }
    }

    std::vector<Chunk *> to_convert;
    std::vector<Chunk *> to_build;

    // Update patch + border
    for (int x = player_chunk.x - render_distance - 1; x <= player_chunk.x + render_distance + 1; x++)
    {
        for (int z = player_chunk.z - render_distance - 1; z <= player_chunk.z + render_distance + 1; z++)
        {
            uint64_t chunk_id = ChunkCoordsToID({x, 0, z});
            bool on_new_border = x == player_chunk.x - render_distance - 1
                                || x == player_chunk.x + render_distance + 1
                                || z == player_chunk.z + render_distance + 1
                                || z == player_chunk.z - render_distance - 1;

            if (chunks_.contains(chunk_id))
            {
                auto chunk = chunks_.at(chunk_id);
                if (on_new_border) // Mark as border chunk
                {
                    chunk->SetIsBorderChunk(true);
                }
                else if (chunk->IsBorderChunk()) // Mark as non-border chunk
                {
                    // We can't build yet, as we haven't guaranteed the existence of all neighbors, so we must defer
                    chunk->SetIsBorderChunk(false);
                    to_convert.push_back(chunk);
                }
            }
            else
            {
                // We can't build yet, as we haven't guaranteed the existence of all neighbors, so we must defer
                Chunk *chunk = new Chunk(glm::ivec3{x, 0, z}, on_new_border, this);
                chunks_.emplace(chunk_id, chunk);
                to_build.push_back(chunk);
            }
        }
    }

    // Build border chunks that just became patch chunks
    for (auto chunk : to_convert)
    {
        chunk->PinNeighbors();
        job_queue_.push({
            .chunk = chunk,
            .requires_neighbors = true,
            .tasks = {
                ChunkTask::BUILD_LIGHTMAP_EXTERNAL,
                ChunkTask::BUILD_VERTICES,
                ChunkTask::MARK_AS_CLEAN
            }
        });
    }

    // Build new patch chunks
    for (auto chunk : to_build)
    {
        if (chunk->IsBorderChunk())
        {
            job_queue_.push({
                .chunk = chunk,
                .requires_neighbors = false,
                .tasks = {
                    ChunkTask::LOAD_BLOCKS,
                    ChunkTask::BUILD_LIGHTMAP_INTERNAL,
                    ChunkTask::MARK_AS_CLEAN
                }
            });
        }
        else
        {
            chunk->PinNeighbors();
            job_queue_.push({
                .chunk = chunk,
                .requires_neighbors = true,
                .tasks = {
                    ChunkTask::LOAD_BLOCKS,
                    ChunkTask::BUILD_LIGHTMAP_INTERNAL,
                    ChunkTask::BUILD_LIGHTMAP_EXTERNAL,
                    ChunkTask::BUILD_VERTICES,
                    ChunkTask::MARK_AS_CLEAN
                }
            });
        }
    }
}

BlockID *ChunkManager::GetBlockMemory(uint64_t chunk_id)
{
    // Check for free memory blocks
    for (auto &memory : block_memory_)
    {
        if (!memory.in_use)
        {
            memory.in_use = true;
            memory.owner = chunk_id;
            return memory.blocks;
        }
    }

    // No free memory blocks; create new one
    BlockMemory new_memory {
        .blocks = (BlockID *)malloc(BLOCKS_IN_CHUNK * sizeof(BlockID)),
        .in_use = true,
        .owner = chunk_id
    };
    block_memory_.push_back(new_memory);
    return new_memory.blocks;
}

void ChunkManager::ReuseBlockMemory(uint64_t chunk_id)
{
    for (auto &memory : block_memory_)
    {
        if (memory.owner == chunk_id)
        {
            memory.in_use = false;
            return;
        }
    }
}

Chunk *ChunkManager::GetChunk(glm::ivec3 chunk_coords)
{
    auto chunk_id = ChunkCoordsToID(chunk_coords);
    if (chunks_.contains(chunk_id))
        return chunks_.at(chunk_id);
    else
        return nullptr;
}

ChunkWorkerPool *ChunkManager::GetWorkerPool()
{
    return worker_pool_;
}

int ChunkManager::GetLoadedChunkCount()
{
    return loaded_chunk_count_;
}

void ChunkManager::WriteAllChunksToDisk()
{
    for (auto &[chunk_id, chunk] : chunks_)
        WriteChunkToDisk(chunk->GetFilePath(), chunk->GetBlocks());
}
