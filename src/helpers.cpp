
#include <cstdlib>
#include <fstream>
#include <vector>

#include "helpers.h"
#include "constants.h"
#include "block.h"

uint64_t ChunkCoordsToID(const glm::ivec3& chunk_coords) noexcept
{
    uint64_t combined = (uint64_t)((uint32_t)chunk_coords.x);
    combined <<= (sizeof(uint64_t) * 8 / 2);
    combined |= (uint64_t)((uint32_t)chunk_coords.z);
    return combined;
}

glm::ivec3 ChunkIDToCoords(uint64_t id) noexcept
{
    int chunk_z = (int)((uint32_t)id);
    id >>= (sizeof(uint64_t) * 8 / 2);
    int chunk_x = (int)((uint32_t)id);
    return glm::ivec3(chunk_x, 0, chunk_z);
}

// Convert arbitrary global position to nearest voxel position
glm::ivec3 GetNearestVoxel(const glm::vec3& global_pos) noexcept
{
    return glm::ivec3(
        glm::round(global_pos.x),
        glm::round(global_pos.y),
        glm::round(global_pos.z)
    );
}

// Get chunk coordinate the voxel position belongs to
glm::ivec3 VoxelToChunk(const glm::ivec3& voxel_pos) noexcept
{
    return glm::ivec3(
        glm::floor((float)voxel_pos.x / (float)CHUNK_SIZE),
        0,
        glm::floor((float)voxel_pos.z / (float)CHUNK_SIZE)
    );
}

// Convert global voxel position to local (in chunk) voxel position
glm::ivec3 GlobalToLocalVoxel(const glm::ivec3& global_voxel_pos) noexcept
{
    glm::ivec3 chunk_coord = VoxelToChunk(global_voxel_pos);
    return glm::ivec3(
        (global_voxel_pos.x - chunk_coord.x * CHUNK_SIZE),
        global_voxel_pos.y,
        (global_voxel_pos.z - chunk_coord.z * CHUNK_SIZE)
    );
}

// Convert local (in chunk) voxel position to global voxel position
glm::ivec3 LocalToGlobalVoxel(const glm::ivec3& local_voxel_pos, const glm::ivec3& chunk_coord) noexcept
{
    return glm::ivec3(
        local_voxel_pos.x + chunk_coord.x * CHUNK_SIZE,
        local_voxel_pos.y,
        local_voxel_pos.z + chunk_coord.z * CHUNK_SIZE
    );
}

bool ChunkInFrustum(const Plane frustum[6], const glm::vec3& chunk_min, const glm::vec3& chunk_max)
{
    for (int i = 0; i < 6; i++)
    {
        const Plane& p = frustum[i];

        // Compute the most positive point relative to plane normal
        glm::vec3 positive = {
            (p.normal.x >= 0 ? chunk_max.x : chunk_min.x),
            (p.normal.y >= 0 ? chunk_max.y : chunk_min.y),
            (p.normal.z >= 0 ? chunk_max.z : chunk_min.z),
        };

        // If that point is behind the plane, the chunk is outside
        if (glm::dot(p.normal, positive) + p.d < 0)
            return false;
    }

    return true;
}

void GetFrustumPlanes(const glm::mat4 &view_proj, Plane *frustum)
{
    int a, b;
    for (int i = 0; i < 6; i++)
    {
        a = i / 2;
        b = 1 - 2*(i & 1);

        frustum[i].normal.x = view_proj[0][3] + view_proj[0][a] * b;
        frustum[i].normal.y = view_proj[1][3] + view_proj[1][a] * b;
        frustum[i].normal.z = view_proj[2][3] + view_proj[2][a] * b;
        frustum[i].d        = view_proj[3][3] + view_proj[3][a] * b;

        float len = glm::length(frustum[i].normal);
        frustum[i].normal /= len;
        frustum[i].d      /= len;
    }
}

uint64_t SplitMix64(uint64_t& x)
{
    x += 0x9E3779B97F4A7C15ULL;
    uint64_t z = x;
    z = (z ^ (z >> 30ULL)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27ULL)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31ULL);
}

//
// Chunk loading/saving
//

struct RLEEntry
{
    uint8_t count;
    BlockID block;
};

// Decodes chunk from disk and writes it to `blocks`
void LoadChunkFromDisk(std::filesystem::path chunk_file_path, BlockID *blocks)
{
    std::streamsize file_size;
    std::ifstream chunk_file(chunk_file_path, std::ios::binary);

    // Get file size
    chunk_file.seekg(0, std::ios::end);
    file_size = chunk_file.tellg();
    chunk_file.seekg(0, std::ios::beg);

    // Pull entries
    RLEEntry entries[file_size / sizeof(RLEEntry)]; // file_size should be an exact multiple of sizeof(RLEEntry)
    chunk_file.read(reinterpret_cast<char *>(entries), file_size);
    chunk_file.close();

    // Write block data
    size_t block_index = 0;
    for (RLEEntry entry : entries)
    {
        while (entry.count > 0)
        {
            blocks[block_index] = entry.block;
            entry.count--;
            block_index++;
        }
    }
}

// Run-length encodes the chunk and writes it to disk
void WriteChunkToDisk(std::filesystem::path chunk_file_path, BlockID *blocks)
{
    std::vector<RLEEntry> entries;
    entries.reserve(8000); // Initially generated chunks typically have 6000-7000. Added some padding.

    uint8_t count = 0;
    BlockID block = blocks[GetChunkIndex(0, 0, 0)];
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < WORLD_HEIGHT_LIMIT; y++)
            {
                BlockID next_block = blocks[GetChunkIndex(x, y, z)];
                if (next_block == block)
                {
                    count++;
                }
                else
                {
                    entries.emplace_back(count, block);
                    count = 1;
                    block = next_block;
                }
            }
        }
    }

    std::ofstream chunk_file(chunk_file_path, std::ios::binary);
    chunk_file.write(reinterpret_cast<const char *>(entries.data()), entries.size() * sizeof(RLEEntry));
    chunk_file.close();
}
