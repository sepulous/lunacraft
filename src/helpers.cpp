
#include <cstdlib>
#include <fstream>
#include <vector>
#include <unordered_map>

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

BlockID ItemIDToBlockID(ItemID item_id)
{
    static auto map = []()
    {
        std::unordered_map<ItemID, BlockID> _map {
            {ItemID::water, BlockID::water},
            {ItemID::sulphur_crystal, BlockID::sulphur_crystal},
            {ItemID::blue_crystal, BlockID::blue_crystal},
            {ItemID::boron_crystal, BlockID::boron_crystal},
            {ItemID::glass, BlockID::glass},
            {ItemID::aluminum, BlockID::aluminum},
            {ItemID::aluminum_ore, BlockID::aluminum_ore},
            {ItemID::amethyst_ore, BlockID::amethyst_ore},
            {ItemID::beacon, BlockID::beacon},
            {ItemID::beryllium, BlockID::beryllium},
            {ItemID::calcite, BlockID::calcite},
            {ItemID::carbon, BlockID::carbon},
            {ItemID::chalchanthite, BlockID::chalchanthite},
            {ItemID::dirt, BlockID::dirt},
            {ItemID::feldspar, BlockID::feldspar},
            {ItemID::gold_ore, BlockID::gold_ore},
            {ItemID::granite, BlockID::granite},
            {ItemID::graphite, BlockID::graphite},
            {ItemID::gravel, BlockID::gravel},
            {ItemID::light, BlockID::light},
            {ItemID::magnetite, BlockID::magnetite},
            {ItemID::molybdenum_ore, BlockID::molybdenum_ore},
            {ItemID::moon_bark, BlockID::moon_bark},
            {ItemID::moon_leaf, BlockID::moon_leaf},
            {ItemID::moon_wood, BlockID::moon_wood},
            {ItemID::neptunium, BlockID::neptunium},
            {ItemID::notchium, BlockID::notchium},
            {ItemID::notchium_ore, BlockID::notchium_ore},
            {ItemID::phosphate, BlockID::phosphate},
            {ItemID::polymer, BlockID::polymer},
            {ItemID::quartz_ore, BlockID::quartz_ore},
            {ItemID::rock, BlockID::rock},
            {ItemID::sand, BlockID::sand},
            {ItemID::shale_gravel, BlockID::shale_gravel},
            {ItemID::silver_ore, BlockID::silver_ore},
            {ItemID::snow, BlockID::snow},
            {ItemID::sulphur_ore, BlockID::sulphur_ore},
            {ItemID::titanium, BlockID::titanium},
            {ItemID::titanium_ore, BlockID::titanium_ore},
            {ItemID::topsoil, BlockID::topsoil},
            {ItemID::xenostone, BlockID::xenostone},
            {ItemID::zircon_ore, BlockID::zircon_ore},
        };

        return _map;
    }();

    if (!map.contains(item_id))
        return BlockID::air;
    else
        return map.at(item_id);
}

ItemID BlockIDToItemID(BlockID block_id)
{
    static auto map = []()
    {
        std::unordered_map<BlockID, ItemID> _map {
            {BlockID::water, ItemID::water},
            {BlockID::sulphur_crystal, ItemID::sulphur_crystal},
            {BlockID::blue_crystal, ItemID::blue_crystal},
            {BlockID::boron_crystal, ItemID::boron_crystal},
            {BlockID::glass, ItemID::glass},
            {BlockID::aluminum, ItemID::aluminum},
            {BlockID::aluminum_ore, ItemID::aluminum_ore},
            {BlockID::amethyst_ore, ItemID::amethyst_ore},
            {BlockID::beacon, ItemID::beacon},
            {BlockID::beryllium, ItemID::beryllium},
            {BlockID::calcite, ItemID::calcite},
            {BlockID::carbon, ItemID::carbon},
            {BlockID::chalchanthite, ItemID::chalchanthite},
            {BlockID::dirt, ItemID::dirt},
            {BlockID::feldspar, ItemID::feldspar},
            {BlockID::gold_ore, ItemID::gold_ore},
            {BlockID::granite, ItemID::granite},
            {BlockID::graphite, ItemID::graphite},
            {BlockID::gravel, ItemID::gravel},
            {BlockID::light, ItemID::light},
            {BlockID::magnetite, ItemID::magnetite},
            {BlockID::molybdenum_ore, ItemID::molybdenum_ore},
            {BlockID::moon_bark, ItemID::moon_bark},
            {BlockID::moon_leaf, ItemID::moon_leaf},
            {BlockID::moon_wood, ItemID::moon_wood},
            {BlockID::neptunium, ItemID::neptunium},
            {BlockID::notchium, ItemID::notchium},
            {BlockID::notchium_ore, ItemID::notchium_ore},
            {BlockID::phosphate, ItemID::phosphate},
            {BlockID::polymer, ItemID::polymer},
            {BlockID::quartz_ore, ItemID::quartz_ore},
            {BlockID::rock, ItemID::rock},
            {BlockID::sand, ItemID::sand},
            {BlockID::shale_gravel, ItemID::shale_gravel},
            {BlockID::silver_ore, ItemID::silver_ore},
            {BlockID::snow, ItemID::snow},
            {BlockID::sulphur_ore, ItemID::sulphur_ore},
            {BlockID::titanium, ItemID::titanium},
            {BlockID::titanium_ore, ItemID::titanium_ore},
            {BlockID::topsoil, ItemID::topsoil},
            {BlockID::xenostone, ItemID::xenostone},
            {BlockID::zircon_ore, ItemID::zircon_ore},
        };

        return _map;
    }();

    if (!map.contains(block_id))
        return ItemID::none;
    else
        return map.at(block_id);
}

bool ChunkInFrustum(const Plane frustum[6], float chunk_min_x, float chunk_min_z)
{
    for (int i = 0; i < 6; i++)
    {
        const Plane& p = frustum[i];

        // Compute the most positive point relative to plane normal
        glm::vec3 positive = {
            (p.normal.x < 0 ? chunk_min_x : chunk_min_x + CHUNK_SIZE),
            (p.normal.y < 0 ? 0           : WORLD_HEIGHT_LIMIT),
            (p.normal.z < 0 ? chunk_min_z : chunk_min_z + CHUNK_SIZE),
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
            block_index++;
            entry.count--;
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
    entries.emplace_back(count, block);

    std::ofstream chunk_file(chunk_file_path, std::ios::binary);
    chunk_file.write(reinterpret_cast<const char *>(entries.data()), entries.size() * sizeof(RLEEntry));
    chunk_file.close();
}
