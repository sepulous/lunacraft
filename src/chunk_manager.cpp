
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
#include "rng.h"
#include "alchemy.h"
#include "dropped_item.h"

//
// ChunkManager
//

ChunkManager::~ChunkManager()
{
    glDeleteTextures(1, &texture_atlas_);
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
    std::filesystem::create_directories(chunk_dir);

    // Reserve for maximum number of possible memory blocks (including border chunks)
    block_memory_.reserve((2*MAX_RENDER_DISTANCE + 3) * (2*MAX_RENDER_DISTANCE + 3));

    // Start worker pool
    worker_pool_ = std::make_unique<ChunkWorkerPool>();
}

void ChunkManager::HandleChunkJobs()
{
    // Entity loading
    for (auto need_entities_it = need_entities_.begin(); need_entities_it != need_entities_.end(); )
    {
        auto chunk_id = *need_entities_it;
        if (auto chunk_it = chunks_.find(chunk_id); chunk_it != chunks_.end())
        {
            auto& chunk = chunk_it->second;
            if (chunk->GetState() >= ChunkState::INTERNAL_DONE)
            {
                Moon::GetCurrentMoon()->GetEntityManager().LoadChunkEntities(chunk->GetCoords());
                need_entities_it = need_entities_.erase(need_entities_it);
            }
            else
            {
                ++need_entities_it;
            }
        }
        else
        {
            need_entities_it = need_entities_.erase(need_entities_it);
        }
    }

    // Worker thread jobs
    const int jobs_to_handle = job_queue_.size(); // Jobs can be requeued, so we should only get this once at the beginning
    for (int i = 0; i < jobs_to_handle; i++)
    {
        if (job_queue_.front().chunk->IsDirty())
            continue;

        auto job = std::move(job_queue_.front());
        job_queue_.pop();

        if (job.requires_neighbors)
        {
            auto neighbors = GetAdjacentNeighbors(job.chunk->GetCoords());
            bool neighbors_ready = std::none_of(neighbors.begin(), neighbors.end(), [](auto neighbor) {
                return neighbor->GetState() < ChunkState::INTERNAL_DONE;
            });

            if (neighbors_ready) // All tasks can be done
            {
                job.chunk->MarkAsDirty();
                worker_pool_->SubmitJob(std::move(job));
            }
            else // External tasks cannot be done, but there may be internal tasks that can
            {
                auto it = std::partition_point(job.tasks.begin(), job.tasks.end(), [](auto task) {
                    return task == ChunkTask::LoadBlocks || task == ChunkTask::BuildLightmapInternal;
                });

                if (it == job.tasks.begin()) // All tasks are external, so just requeue the job
                {
                    job_queue_.push(std::move(job));
                }
                else
                {
                    job.chunk->MarkAsDirty();

                    // Submit internal tasks
                    std::vector<ChunkTaskFn> internal_tasks{job.tasks.begin(), it};
                    internal_tasks.push_back(ChunkTask::MarkAsClean);
                    worker_pool_->SubmitJob({std::move(internal_tasks), job.chunk, job.requires_neighbors});

                    // Requeue remaining external tasks
                    job.tasks = std::vector<ChunkTaskFn>{it, job.tasks.end()};
                    job_queue_.push(std::move(job));
                }
            }
        }
        else // Purely internal job; just submit everything
        {
            job.chunk->MarkAsDirty();
            worker_pool_->SubmitJob(std::move(job));
        }
    }
}

void ChunkManager::UploadReadyChunks()
{
    for (auto& [chunk_id, chunk] : chunks_)
    {
        if (!chunk->IsBorderChunk() && chunk->GetState() == ChunkState::READY_TO_UPLOAD) // Shouldn't waste GPU memory with border chunks. Most are never rendered.
        {
            chunk->UploadVertices();
            loaded_chunk_count_++;
        }
    }
}

void ChunkManager::HandlePlayerModification(glm::ivec3 voxel, BlockID block_placed)
{
    if (voxel.y <= 0 || voxel.y >= WORLD_HEIGHT_LIMIT)
        return;

    // Get chunk
    const auto chunk_coords = VoxelToChunk(voxel);
    auto& chunk = chunks_.at(ChunkCoordsToID(chunk_coords));

    // Remove block
    const auto local = GlobalToLocalVoxel(voxel);
    chunk->GetBlocks()[GetChunkIndex(local)] = block_placed;

    // Rebuild this chunk
    chunk->PinAllNeighbors();
    job_queue_.push({
        .tasks = {
            ChunkTask::BuildLightmapInternal,
            ChunkTask::BuildLightmapExternal,
            ChunkTask::BuildVertices,
            ChunkTask::UnpinAllNeighbors,
            ChunkTask::MarkAsClean,
        },
        .chunk = chunk.get(),
        .requires_neighbors = true,
    });

    // Since the maximum light level is 9, only blocks that are up to 9 blocks away can be affected by
    // chunk modifications, so neighbor chunks outside this range don't need to be updated. At the same
    // time, it's not always the case that all neighbors in this range need to be updated, so this can
    // be more optimized.
    glm::ivec3 light_extents[] = {
        voxel + glm::ivec3{-9, 0,  9},
        voxel + glm::ivec3{ 9, 0,  9},
        voxel + glm::ivec3{ 9, 0, -9},
        voxel + glm::ivec3{-9, 0, -9},
    };

    // Rebuild neighbor chunks
    auto neighbor_chunks = GetAllNeighbors(chunk_coords);
    for (auto neighbor : neighbor_chunks)
    {
        for (auto& light_extent : light_extents)
        {
            auto extent_chunk_coord = VoxelToChunk(light_extent);
            if (neighbor->GetCoords() == extent_chunk_coord)
            {
                neighbor->PinAllNeighbors();
                job_queue_.push({
                    .tasks = {
                        ChunkTask::BuildLightmapInternal,
                        ChunkTask::BuildLightmapExternal,
                        ChunkTask::BuildVertices,
                        ChunkTask::UnpinAllNeighbors,
                        ChunkTask::MarkAsClean,
                    },
                    .chunk = neighbor,
                    .requires_neighbors = true,
                });
                break;
            }
        }
    }
}

void ChunkManager::HandleBrownMobExplosion(glm::ivec3 explosion_center)
{
    // Determine which voxels to destroy
    std::vector<glm::ivec3> to_destroy;
    to_destroy.reserve(256);
    for (int dx = -3; dx <= 3; dx++)
    {
        for (int dz = -3; dz <= 3; dz++)
        {
            for (int dy = -3; dy <= 3; dy++)
            {
                glm::ivec3 voxel = explosion_center + glm::ivec3{dx, dy, dz};
                float distance_from_center = glm::distance(glm::vec3{voxel}, glm::vec3{explosion_center});
                if (voxel.y > 0 && voxel.y < WORLD_HEIGHT_LIMIT && (distance_from_center <= 2.5f || RNG{}.Range(1, 10) <= 6))
                    to_destroy.push_back(voxel);
            }
        }
    }

    // Break and convert (alchemy) blocks, and record which chunks need to be updated
    std::vector<glm::ivec3> chunks_affected;
    BlockID batch_block = BlockID::air;
    int batch_count = 0;
    for (auto it = to_destroy.begin(); it != to_destroy.end(); ++it)
    {
        auto& voxel = *it;

        glm::ivec3 affected_coords[] = {
            VoxelToChunk(voxel),
            VoxelToChunk(voxel + glm::ivec3{ 1, 0,  0}),
            VoxelToChunk(voxel + glm::ivec3{-1, 0,  0}),
            VoxelToChunk(voxel + glm::ivec3{ 0, 0,  1}),
            VoxelToChunk(voxel + glm::ivec3{ 0, 0, -1}),
        };

        // Record affected chunks
        for (auto& coords : affected_coords)
        {
            auto chunk_id = ChunkCoordsToID(coords);
            if (!chunks_.contains(chunk_id))
                continue;

            if (std::find(chunks_affected.begin(), chunks_affected.end(), coords) == chunks_affected.end())
                chunks_affected.push_back(coords);
        }

        // Break/convert voxel
        auto& chunk = chunks_.at(ChunkCoordsToID(VoxelToChunk(voxel)));
        BlockID& block = chunk->GetBlocks()[GetChunkIndex(GlobalToLocalVoxel(voxel))];
        if (block != BlockID::air)
        {
            BlockID block_to_drop;
            if (block == BlockID::topsoil)
                block_to_drop = BlockID::dirt;
            else if (RNG{}.Range(1, 5) == 1) // 20% chance of alchemy
                block_to_drop = GetAlchemyProduct(block);
            else
                block_to_drop = block;

            block = BlockID::air;

            if (batch_block == BlockID::air)
            {
                batch_block = block_to_drop;
                batch_count = 1;
            }
            else if (block_to_drop == batch_block)
            {
                batch_count++;
            }

            if (block_to_drop != batch_block || it == to_destroy.end() - 1)
            {
                DroppedItem *item = new DroppedItem({
                    .position = glm::vec3{voxel},
                    .item = BlockIDToItemID(batch_block),
                    .amount = batch_count
                });
                item->SetVelocity({
                    RNG{}.Range(-1.0f, 1.0f),
                    RNG{}.Range(0.5f, 1.0f),
                    RNG{}.Range(-1.0f, 1.0f)
                });
                Moon::GetCurrentMoon()->GetEntityManager().AddEntity(item);

                batch_block = block_to_drop;
                batch_count = 1;
            }
        }
        else if (it == to_destroy.end() - 1)
        {
            DroppedItem *item = new DroppedItem({
                .position = glm::vec3{voxel},
                .item = BlockIDToItemID(batch_block),
                .amount = batch_count
            });
            item->SetVelocity({
                RNG{}.Range(-1.0f, 1.0f),
                RNG{}.Range(0.5f, 1.0f),
                RNG{}.Range(-1.0f, 1.0f)
            });
            Moon::GetCurrentMoon()->GetEntityManager().AddEntity(item);
        }
    }

    // Update chunks
    for (auto& chunk_coords : chunks_affected)
    {
        auto& chunk = chunks_.at(ChunkCoordsToID(chunk_coords));
        chunk->PinAllNeighbors();
        job_queue_.push({
            .tasks = {
                ChunkTask::BuildLightmapInternal,
                ChunkTask::BuildLightmapExternal,
                ChunkTask::BuildVertices,
                ChunkTask::UnpinAllNeighbors,
                ChunkTask::MarkAsClean,
            },
            .chunk = chunk.get(),
            .requires_neighbors = true,
        });
    }
}

void ChunkManager::RenderChunks(Plane frustum[6])
{
    glBindTexture(GL_TEXTURE_2D, texture_atlas_);
    glDepthFunc(GL_LESS);

    std::vector<Chunk*> visible_chunks;
    visible_chunks.reserve(32);

    // Render opaque blocks
    for (auto it = chunks_.begin(); it != chunks_.end(); ++it)
    {
        auto& chunk = it->second;
        if (chunk->HasUploadedVertices() && !chunk->IsBorderChunk())
        {
            glm::ivec3 chunk_coords = chunk->GetCoords();
            float x0 = chunk_coords.x * CHUNK_SIZE;
            float z0 = chunk_coords.z * CHUNK_SIZE;
            if (ChunkInFrustum(frustum, x0, z0))
            {
                chunk->RenderOpaques();
                visible_chunks.push_back(chunk.get());
            }
        }
    }

    // Render transparent blocks
    for (auto chunk : visible_chunks)
    {
        chunk->RenderTransparents();
    }
}

void ChunkManager::UpdateGlobalLighting()
{
    for (auto& [chunk_id, chunk] : chunks_)
    {
        if (!chunk->IsBorderChunk())
        {
            chunk->PinAdjacentNeighbors();
            job_queue_.push({
                .tasks = {
                    ChunkTask::UpdateVertexLighting,
                    ChunkTask::UnpinAdjacentNeighbors,
                    ChunkTask::MarkAsClean,
                },
                .chunk = chunk.get(),
                .requires_neighbors = true,
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
    const auto player_chunk = VoxelToChunk(GetNearestVoxel(Moon::GetCurrentMoon()->GetPlayer()->GetPosition()));
    const int render_distance = OptionsManager::GetOptions().render_distance;
    for (int x = player_chunk.x - render_distance - 1; x <= player_chunk.x + render_distance + 1; x++)
    {
        for (int z = player_chunk.z - render_distance - 1; z <= player_chunk.z + render_distance + 1; z++)
        {
            auto chunk_id = ChunkCoordsToID({x, 0, z});
            bool is_border_chunk = x == player_chunk.x - render_distance - 1
                                || x == player_chunk.x + render_distance + 1
                                || z == player_chunk.z - render_distance - 1
                                || z == player_chunk.z + render_distance + 1;

            chunks_.emplace(chunk_id, std::make_unique<Chunk>(glm::ivec3{x, 0, z}, is_border_chunk, this));
        }
    }

    // Chunks expect their neighbors to exist when building, so we defer it
    for (auto& [chunk_id, chunk] : chunks_)
    {
        if (chunk->IsBorderChunk())
        {
            job_queue_.push({
                .tasks = {
                    ChunkTask::LoadBlocks,
                    ChunkTask::BuildLightmapInternal,
                    ChunkTask::MarkAsClean
                },
                .chunk = chunk.get(),
                .requires_neighbors = false,
            });
        }
        else
        {
            chunk->PinAllNeighbors();
            job_queue_.push({
                .tasks = {
                    ChunkTask::LoadBlocks,
                    ChunkTask::BuildLightmapInternal,
                    ChunkTask::BuildLightmapExternal,
                    ChunkTask::BuildVertices,
                    ChunkTask::UnpinAllNeighbors,
                    ChunkTask::MarkAsClean,
                },
                .chunk = chunk.get(),
                .requires_neighbors = true,
            });
        }
    }
}

// Get chunk's adjcent neighbors in order {front, right, back, left}
std::array<Chunk*, 4> ChunkManager::GetAdjacentNeighbors(glm::ivec3 chunk_coords)
{
    std::array<Chunk*, 4> neighbors;

    const glm::ivec3 neighbor_coords[] = {
        {chunk_coords.x + 0, 0, chunk_coords.z + 1}, // Front
        {chunk_coords.x + 1, 0, chunk_coords.z + 0}, // Right
        {chunk_coords.x + 0, 0, chunk_coords.z - 1}, // Back
        {chunk_coords.x - 1, 0, chunk_coords.z + 0}  // Left
    };

    size_t idx = 0;
    for (auto& neighbor : neighbor_coords)
    {
        auto chunk_id = ChunkCoordsToID(neighbor);
        neighbors[idx] = chunks_.at(chunk_id).get(); // Existence of neighbors should be guaranteed
        idx++;
    }

    return neighbors;
}

// Get all chunk neighbors in order {front, right, back, left, front_right, front_left, back_right, back_left}
std::array<Chunk*, 8> ChunkManager::GetAllNeighbors(glm::ivec3 chunk_coords)
{
    std::array<Chunk*, 8> neighbors;

    const glm::ivec3 neighbor_coords[] = {
        {chunk_coords.x + 0, 0, chunk_coords.z + 1}, // Front
        {chunk_coords.x + 1, 0, chunk_coords.z + 0}, // Right
        {chunk_coords.x + 0, 0, chunk_coords.z - 1}, // Back
        {chunk_coords.x - 1, 0, chunk_coords.z + 0}, // Left
        {chunk_coords.x + 1, 0, chunk_coords.z + 1}, // Front right
        {chunk_coords.x - 1, 0, chunk_coords.z + 1}, // Front left
        {chunk_coords.x + 1, 0, chunk_coords.z - 1}, // Back right
        {chunk_coords.x - 1, 0, chunk_coords.z - 1}, // Back left
    };

    size_t idx = 0;
    for (auto& neighbor : neighbor_coords)
    {
        auto chunk_id = ChunkCoordsToID(neighbor);
        neighbors[idx] = chunks_.at(chunk_id).get(); // Existence of neighbors should be guaranteed
        idx++;
    }

    return neighbors;
}

void ChunkManager::AdjustChunkPatch()
{
    const auto player_chunk = VoxelToChunk(GetNearestVoxel(Moon::GetCurrentMoon()->GetPlayer()->GetPosition()));
    const auto render_distance = OptionsManager::GetOptions().render_distance;

    // Remove all chunks outside the patch + border
    for (auto it = chunks_.begin(); it != chunks_.end(); )
    {
        auto& chunk = it->second;

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
                WriteChunkToDisk(chunk->GetFilePath(), chunk->GetBlocks());
                ReuseBlockMemory(chunk->GetID());

                Moon::GetCurrentMoon()->GetEntityManager().UnloadChunkEntities(coords);

                // Erase chunk
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

            if (auto chunk_it = chunks_.find(chunk_id); chunk_it != chunks_.end())
            {
                auto& chunk = chunk_it->second;
                if (on_new_border) // Convert to border chunk
                {
                    Moon::GetCurrentMoon()->GetEntityManager().UnloadChunkEntities(chunk->GetCoords());
                    chunk->SetIsBorderChunk(true);
                }
                else if (chunk->IsBorderChunk()) // Mark as non-border chunk
                {
                    // We can't build yet, as we haven't guaranteed the existence of all neighbors, so we must defer
                    chunk->SetIsBorderChunk(false);
                    to_convert.push_back(chunk.get());
                }
            }
            else
            {
                // We can't build yet, as we haven't guaranteed the existence of all neighbors, so we must defer
                auto [new_chunk_it, _] = chunks_.emplace(chunk_id, std::make_unique<Chunk>(glm::ivec3{x, 0, z}, on_new_border, this));
                to_build.push_back(new_chunk_it->second.get());
            }
        }
    }

    // Build border chunks that just became patch chunks
    for (auto chunk : to_convert)
    {
        chunk->PinAllNeighbors();
        job_queue_.push({
            .tasks = {
                ChunkTask::BuildLightmapExternal,
                ChunkTask::BuildVertices,
                ChunkTask::UnpinAllNeighbors,
                ChunkTask::MarkAsClean,
            },
            .chunk = chunk,
            .requires_neighbors = true,
        });

        need_entities_.push_back(ChunkCoordsToID(chunk->GetCoords()));
    }

    // Build new patch chunks
    for (auto chunk : to_build)
    {
        if (chunk->IsBorderChunk())
        {
            job_queue_.push({
                .tasks = {
                    ChunkTask::LoadBlocks,
                    ChunkTask::BuildLightmapInternal,
                    ChunkTask::MarkAsClean
                },
                .chunk = chunk,
                .requires_neighbors = false,
            });
        }
        else
        {
            chunk->PinAllNeighbors();
            job_queue_.push({
                .tasks = {
                    ChunkTask::LoadBlocks,
                    ChunkTask::BuildLightmapInternal,
                    ChunkTask::BuildLightmapExternal,
                    ChunkTask::BuildVertices,
                    ChunkTask::UnpinAllNeighbors,
                    ChunkTask::MarkAsClean,
                },
                .chunk = chunk,
                .requires_neighbors = true,
            });
        }

        need_entities_.push_back(ChunkCoordsToID(chunk->GetCoords()));
    }
}

BlockID* ChunkManager::GetBlockMemory(uint64_t chunk_id)
{
    // Check for free memory blocks
    for (auto& memory : block_memory_)
    {
        if (!memory.in_use)
        {
            memory.in_use = true;
            memory.owner = chunk_id;
            return memory.blocks.get();
        }
    }

    // No free memory blocks; create new one
    auto& new_memory = block_memory_.emplace_back(
        std::make_unique<BlockID[]>(BLOCKS_IN_CHUNK),
        chunk_id,
        true // in use
    );
    return new_memory.blocks.get();
}

void ChunkManager::ReuseBlockMemory(uint64_t chunk_id)
{
    for (auto& memory : block_memory_)
    {
        if (memory.owner == chunk_id)
        {
            memory.in_use = false;
            return;
        }
    }
}

std::vector<Chunk*> ChunkManager::GetAllChunks()
{
    std::vector<Chunk*> chunks;
    chunks.reserve(chunks_.size());
    for (auto& [chunk_id, chunk] : chunks_)
        chunks.push_back(chunk.get());

    return chunks;
}

Chunk* ChunkManager::GetChunk(glm::ivec3 chunk_coords)
{
    auto chunk_id = ChunkCoordsToID(chunk_coords);
    if (auto it = chunks_.find(chunk_id); it != chunks_.end())
    {
        auto& chunk = it->second;
        return chunk.get();
    }
    else
    {
        return nullptr;
    }
}

BlockID ChunkManager::GetBlockAt(glm::ivec3 voxel)
{
    auto chunk = GetChunk(VoxelToChunk(voxel));
    if (chunk)
        return chunk->GetBlocks()[GetChunkIndex(GlobalToLocalVoxel(voxel))];
    else
        return BlockID::air;
}

ChunkWorkerPool* ChunkManager::GetWorkerPool()
{
    return worker_pool_.get();
}

int ChunkManager::GetLoadedChunkCount()
{
    return loaded_chunk_count_;
}

void ChunkManager::WriteAllChunksToDisk()
{
    for (auto& [chunk_id, chunk] : chunks_)
        WriteChunkToDisk(chunk->GetFilePath(), chunk->GetBlocks());
}
