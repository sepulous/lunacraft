
#include <vector>
#include <cstdlib>

#include <glm/glm.hpp>

#include "chunk.h"
#include "constants.h"
#include "simplex_noise.h"
#include "helpers.h"
#include "rng.h"
#include "chunk_generation.h"
#include "moon.h"

static void GenerateCrystal(BlockID *, RNG &, float, int, int, int, int, BlockID, int, int);
static void GenerateGreenTreeBranch(BlockID *, RNG &, float, int, int, int);
static void GenerateSpiralTreeBranch(BlockID *, RNG &, float, int, int, int);
static void GenerateColorTreeBranch(BlockID *, RNG &, float, int, int, int);
static void GenerateVein(BlockID *, RNG &, float, float, float, float, float, int, BlockID, int);
static void GenerateLairShaft(BlockID *, RNG &, int, int, int, int, int, int, int);

static void GenerateHeightMap(uint8_t *height_map, int chunk_x, int chunk_z, uint64_t seed)
{
    //
    // Parameters
    //
    constexpr float SCALE = 1.0f / (float)CHUNK_SIZE;
    constexpr int OCTAVES = 4;
    constexpr float BASE_FREQUENCY = 0.3f;
    constexpr float BASE_AMPLITUDE = 1.0f;
    constexpr float PERSISTENCE = 0.7f;
    constexpr float FREQUENCY_FACTOR = 2.5f;

    //
    // Constants
    //
    constexpr float FREQUENCIES[OCTAVES] = {
        BASE_FREQUENCY,
        BASE_FREQUENCY * FREQUENCY_FACTOR,
        BASE_FREQUENCY * FREQUENCY_FACTOR * FREQUENCY_FACTOR,
        BASE_FREQUENCY * FREQUENCY_FACTOR * FREQUENCY_FACTOR * FREQUENCY_FACTOR
    };
    constexpr float AMPLITUDES[OCTAVES] = {
        BASE_AMPLITUDE,
        BASE_AMPLITUDE * PERSISTENCE,
        BASE_AMPLITUDE * PERSISTENCE * PERSISTENCE,
        BASE_AMPLITUDE * PERSISTENCE * PERSISTENCE * PERSISTENCE
    };
    constexpr float MAX_AMPLITUDE = [&]() { // sum of amplitudes, since max of simplex is 1
        float sum = 0;
        for (int i = 0; i < OCTAVES; i++) sum += AMPLITUDES[i];
        return sum;
    }();
    const double SEED_OFFSET_X = (double)(SplitMix64(seed) & 0xFFFFFFFF);
    const double SEED_OFFSET_Z = (double)(SplitMix64(seed) & 0xFFFFFFFF);

    //
    // Generation
    //

    float float_heights[CHUNK_SIZE * CHUNK_SIZE];

    // Base height map
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            float height = 0.0f;
            for (int i = 0; i < OCTAVES; i++)
            {
                double x_arg = (((x + chunk_x * CHUNK_SIZE) + SEED_OFFSET_X) * SCALE) * FREQUENCIES[i];
                double z_arg = (((z + chunk_z * CHUNK_SIZE) + SEED_OFFSET_Z) * SCALE) * FREQUENCIES[i];
                height += SimplexNoise(x_arg, z_arg) * AMPLITUDES[i];
            }

            float_heights[z + CHUNK_SIZE * x] = height;
        }
    }

    // Normalize
    for (float &height : float_heights)
        height = (height + MAX_AMPLITUDE) / (2*MAX_AMPLITUDE);

    // Scale
    int index = 0;
    for (float height : float_heights)
    {
        float final_height = height * 36.0f + 48.0f;
        height_map[index++] = (uint8_t)final_height;
    }
}

static void ErodeByteTerrain(uint8_t *height_map, float alpha)
{
    // Left to right
    for (int z = 0; z < CHUNK_SIZE; z++)
    {
        float filtered = height_map[z];
        for (int x = 1; x < CHUNK_SIZE; x++)
        {
            uint8_t &height = height_map[z + CHUNK_SIZE * x];
            filtered = alpha * filtered + (1 - alpha) * (float)height;
            height = (uint8_t)filtered;
        }
    }

    // Right to left
    for (int z = 0; z < CHUNK_SIZE; z++)
    {
        float filtered = height_map[z + CHUNK_SIZE * (CHUNK_SIZE - 1)];
        for (int x = CHUNK_SIZE - 2; x >= 0; x--)
        {
            uint8_t &height = height_map[z + CHUNK_SIZE * x];
            filtered = alpha * filtered + (1 - alpha) * (float)height;
            height = (uint8_t)filtered;
        }
    }

    // Top to bottom
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        float filtered = height_map[(CHUNK_SIZE - 1) + CHUNK_SIZE * x];
        for (int z = CHUNK_SIZE - 2; z >= 0; z--)
        {
            uint8_t &height = height_map[z + CHUNK_SIZE * x];
            filtered = alpha * filtered + (1 - alpha) * (float)height;
            height = (uint8_t)filtered;
        }
    }

    // Bottom to top
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        float filtered = height_map[0 + CHUNK_SIZE * x];
        for (int z = 1; z < CHUNK_SIZE; z++)
        {
            uint8_t &height = height_map[z + CHUNK_SIZE * x];
            filtered = alpha * filtered + (1 - alpha) * (float)height;
            height = (uint8_t)filtered;
        }
    }
}

void GenerateChunk(BlockID *chunk, int chunk_x, int chunk_z, MoonSettings settings)
{
    // Seed RNG from chunk coords so structure placement is deterministic
    uint64_t structure_seed = settings.seed ^ ((uint64_t)chunk_x * 73856093ull) ^ ((uint64_t)chunk_z * 19349663ull);
    RNG rng{structure_seed};

    //
    // Generate base terrain
    //

    uint8_t height_map[CHUNK_SIZE * CHUNK_SIZE];
    GenerateHeightMap(height_map, chunk_x, chunk_z, settings.seed);
    ErodeByteTerrain(height_map, glm::mix(0.6f, 0.02f, settings.terrain_roughness));

    //
    // Crater(?)
    //

    bool spawn_crater = rng.Range(0.0f, 1.0f) < 0.18f; // 18% chance (to match original average crater density)
    if (spawn_crater)
    {
        int radius = rng.Range(8, 15);

        float outer_radius = (float)radius;
        float inner_radius = outer_radius * 0.75f;

        int center_x = rng.Range(radius, (CHUNK_SIZE - 1) - radius);
        int center_z = rng.Range(radius, (CHUNK_SIZE - 1) - radius);

        for (int x = center_x - radius; x <= center_x + radius; x++)
        {
            for (int z = center_z - radius; z <= center_z + radius; z++)
            {
                int dx = x - center_x;
                int dz = z - center_z;

                float dist = glm::sqrt((float)(dx*dx + dz*dz));

                if (dist > outer_radius) continue;

                float t;
                if (dist < inner_radius)
                {
                    float u = dist / inner_radius;
                    t = u * u;
                }
                else
                {
                    t = 1.0f - (dist - inner_radius) / (outer_radius - inner_radius);
                }

                float h = height_map[z + CHUNK_SIZE * x];
                h += (outer_radius * t) / 4.0f;

                height_map[z + CHUNK_SIZE * x] = (uint8_t)glm::min(h, 126.0f);
            }
        }
    }

    //
    // Layer determination
    //

    int chunk_index = 0;
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            int height = height_map[z + CHUNK_SIZE * x];
            
            for (int y = 0; y < WORLD_HEIGHT_LIMIT; y++, chunk_index++)
            {
                if (y < height - 8)
                {
                    chunk[chunk_index] = BlockID::rock;
                }
                else if (y < height - 3) // 5 gravel layers
                {
                    chunk[chunk_index] = BlockID::gravel;
                }
                else if (y < height) // 3 dirt layers
                {
                    chunk[chunk_index] = BlockID::dirt;
                }
                else if (y < GROUND_LEVEL && y < height + 3)
                {
                    chunk[chunk_index] = BlockID::sand;
                }
                else if (y >= GROUND_LEVEL || y < height + 3)
                {
                    if (y == height)
                        chunk[chunk_index] = BlockID::topsoil;
                    else if (y > height)
                        chunk[chunk_index] = BlockID::air;
                }
                else
                {
                    chunk[chunk_index] = BlockID::water;
                }
            }
        }
    }

    //
    // Dirt/gravel blobs
    //

    int blob_count = rng.Range(0, 200);
    blob_count *= (float)(CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT_LIMIT) / glm::pow(128.0f, 3.0f); // Scale to match original game's density
    for (int i = 0; i < blob_count; i++)
    {
        int x = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int z = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int y_max = height_map[z + CHUNK_SIZE * x] - 6;
        int y = rng.Range(5, y_max);
        BlockID block = rng.Range(0.0, 1.0) <= 0.5 ? BlockID::gravel : BlockID::dirt;

        GenerateVein(
            chunk,
            rng,
            x, y, z,
            rng.Range(1.0, 5.0), // radius
            rng.Range(0.0, 6.2831855), // pitch
            rng.Range(5, 26), // length
            block,
            3 // recursion depth
        );
    }

    //
    // Ores
    //

    constexpr BlockID MINERALS[] = {BlockID::shale_gravel, BlockID::magnetite, BlockID::aluminum_ore, BlockID::titanium_ore, BlockID::gold_ore, BlockID::notchium_ore, BlockID::blue_crystal};
    constexpr int WEIGHTS[] = {160, 120, 100, 90, 60, 40, 10};
    constexpr int TOTAL_WEIGHT = [&WEIGHTS]()
    {
        int total_weight = 0;
        for (int i = 0; i <= 6; i++) total_weight += WEIGHTS[i];
        return total_weight;
    }();

    int vein_count = rng.Range(0.0f, 1.0f) * rng.Range(0.0f, 1.0f) * 900 + 100;
    vein_count *= (float)(CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT_LIMIT) / glm::pow(128.0f, 3.0f); // Scale to match original game's density
    for (int i = 0; i < vein_count; i++)
    {
        // For the loop right below, weight_cutoff < total_weight, so there's at least
        // one iteration. This ensures that the index is never less than 1, so shale
        // gravel can't spawn. Either this is a bug, or the point is just to add a null
        // contribution to shift the total weight, and thus control the ore distribution.
        int weight_cutoff = rng.Range(0, TOTAL_WEIGHT - 1);
        int remaining = TOTAL_WEIGHT;
        int index = 0;
        for (int j = 0; j < 7 && remaining >= weight_cutoff; j++)
        {
            remaining -= WEIGHTS[j];
            index++;
        }
        index = glm::min(index, 6);

        int weight = WEIGHTS[index];
        BlockID mineral = MINERALS[index];
        int mineral_count = weight * 0.125f * rng.Range(0.0f, 1.0f) + 3.0f;

        int x = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int z = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        uint8_t surface = height_map[z + CHUNK_SIZE * x];
        int y = rng.Range(5, surface + 5);

        GenerateVein(
            chunk,
            rng,
            x, y, z,
            0.5f, // radius
            rng.Range(0.0f, 6.2831855f), // pitch
            mineral_count,
            mineral,
            3 // recursion depth
        );
    }

    //
    // Trees
    //

    // Green trees
    int green_tree_count = settings.tree_cover * (rng.Range(0.0f, 1.0f) * rng.Range(0.0f, 1.0f) - 0.1f) * 140.0f; // max: 126, avg: 63 (my avg: 3.9375)
    green_tree_count *= (float)(CHUNK_SIZE * CHUNK_SIZE) / (float)(128 * 128);
    for (int i = 0; i < green_tree_count; i++)
    {
        int x = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int z = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int y = height_map[z + CHUNK_SIZE * x];
        if (chunk[GetChunkIndex(x, y, z)] == BlockID::topsoil)
        {
            int num_branches = rng.Range(2, 4);
            float angle = rng.Range(0.0f, 6.2831855f);
            for (int j = 0; j < num_branches; j++)
            {
                angle += 6.2831855f / (float)num_branches;
                GenerateGreenTreeBranch(chunk, rng, angle, x, y - 1, z);
            }
        }
    }

    // Spiral trees
    int spiral_tree_count = settings.tree_cover * (rng.Range(0.0f, 1.0f) * rng.Range(0.0f, 1.0f) - 0.2f) * 100.0f; // max: 80, avg: 40 (my avg: 2.5)
    spiral_tree_count *= (float)(CHUNK_SIZE * CHUNK_SIZE) / (float)(128 * 128);
    for (int i = 0; i < spiral_tree_count; i++)
    {
        int x = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int z = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int y = height_map[z + CHUNK_SIZE * x];
        if (chunk[GetChunkIndex(x, y, z)] == BlockID::topsoil)
        {
            int num_branches = rng.Range(2, 4);
            float angle = rng.Range(0.0f, 6.2831855f);
            for (int j = 0; j < num_branches; j++)
            {
                angle += 6.2831855f / (float)num_branches;
                GenerateSpiralTreeBranch(chunk, rng, angle, x, y, z);
            }
        }
    }

    // Color trees
    int color_tree_count = settings.tree_cover * (rng.Range(0.0f, 1.0f) * rng.Range(0.0f, 1.0f) - 0.4f) * 80.0f; // max: 48, avg: 24 (my avg: 1.5)
    color_tree_count *= (float)(CHUNK_SIZE * CHUNK_SIZE) / (float)(128 * 128);
    for (int i = 0; i < color_tree_count; i++)
    {
        int x = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int z = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int y = height_map[z + CHUNK_SIZE * x];
        if (chunk[GetChunkIndex(x, y, z)] == BlockID::topsoil)
        {
            int num_branches = rng.Range(2, 4);
            float angle = rng.Range(0.0f, 6.2831855f);
            for (int j = 0; j < num_branches; j++)
            {
                angle += 6.2831855f / (float)num_branches;
                GenerateColorTreeBranch(chunk, rng, angle, x, y, z);
            }
        }
    }

    //
    // Crystal plants
    //

    constexpr int CRYSTAL_WEIGHTS[] = {65, 30, 5};
    constexpr int TOTAL_CRYSTAL_WEIGHT = 100;
    constexpr BlockID CRYSTALS[] = {BlockID::sulphur_crystal, BlockID::boron_crystal, BlockID::blue_crystal};

    bool spawn_crystal = rng.Range(0.0f, 1.0f) < 0.05f;
    if (spawn_crystal)
    {
        int rand_weight = rng.Range(0, TOTAL_CRYSTAL_WEIGHT - 1);
        int remaining_weight = TOTAL_CRYSTAL_WEIGHT;

        int index = 0;
        while (remaining_weight > rand_weight)
        {
            remaining_weight -= CRYSTAL_WEIGHTS[index];
            if (remaining_weight > rand_weight)
                index++;
        }

        int x = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int z = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int y = height_map[z + CHUNK_SIZE * x];

        float w = rng.Range(2.0, 4.0);
        GenerateCrystal(
            chunk,
            rng,
            w,
            x,
            z,
            y,
            rng.Range(2, 4),
            CRYSTALS[index],
            (int)w,
            4
        );
    }

    //
    // Astronaut lair(?)
    //

    float lair_chance = rng.Range(0.0f, 1.0f);
    bool spawn_lair = lair_chance < (0.1f *((float)CHUNK_SIZE / 128.0f));
    if (spawn_lair)
    {
        int x = CHUNK_SIZE / 2;
        int z = CHUNK_SIZE / 2;
        int y = height_map[z + CHUNK_SIZE * x] + 1;
        GenerateLairShaft(
            chunk,
            rng,
            3,
            x,
            z,
            y,
            15,
            3,
            5
        );
    }
}

//
// This was reverse-engineered from Lunacraft v2.01
//
static void GenerateCrystal(BlockID *chunk, RNG &rng, float radius, int base_x, int base_z, int base_y, int segment_length, BlockID crystal_type, int recursion_depth, int direction)
{
    constexpr int directions[] = {
    //  x   z   y
        0, -1,  0,
        1,  0,  0,
        0,  1,  0,
       -1,  0,  0,
        0,  0,  1,
        0,  0, -1,
    };

    constexpr int direction_map[6][4] = { // Allowed next directions for each direction
        {1, 3, 4, 5},
        {0, 2, 4, 5},
        {1, 3, 4, 5},
        {0, 2, 4, 5},
        {0, 1, 2, 3},
        {0, 1, 2, 3},
    };

    if (segment_length <= 0 || radius < 0.1f || recursion_depth <= 0)
        return;

    int voxel_x = base_x;
    int voxel_z = base_z;
    int voxel_y = base_y;

    int dx = directions[direction * 3];
    int dz = directions[direction * 3 + 1];
    int dy = directions[direction * 3 + 2];

    for (int i = 0; i < segment_length; i++)
    {
        voxel_x += dx;
        voxel_z += dz;
        voxel_y += dy;

        if (BlockIsInChunk(voxel_x, voxel_y, voxel_z))
        {
            BlockID &current_block = chunk[GetChunkIndex(voxel_x, voxel_y, voxel_z)];
            if (current_block == BlockID::air)
                current_block = crystal_type;
        }
    }

    if (radius > 0.0f)
    {
        int branches = static_cast<int>(radius);

        for (int i = 1; i < branches; i++)
        {
            int index = (i + rng.Range(0, 3)) % 4;
            int next_direction = direction_map[direction][index];

            GenerateCrystal(
                chunk,
                rng,
                radius * 0.8f,
                voxel_x,
                voxel_z,
                voxel_y,
                segment_length - 1,
                crystal_type,
                recursion_depth - 1,
                next_direction
            );
        }
    }
}

//
// This code was reverse-engineered from Lunacraft v2.01
//
static void GenerateGreenTreeBranch(BlockID *chunk, RNG &rng, float angle, int base_x, int base_y, int base_z)
{
    float x = (float)base_x;
    float z = (float)base_z;
    float y = (float)base_y;
    float fVar13 = 0.5f * glm::cos(angle);
    float fVar7 = 0.5f * glm::sin(angle);
    float fVar9 = 2.5f * rng.Range(0.8f, 1.2f) * glm::sin(rng.Range(-0.2f, 0.2f) + 2.0943952f);
    float fVar8 = (float)rng.Range(12, 17);
    float dir_x, dir_y, dir_z;
    float x_step, y_step, z_step;

    do
    {
        bool bVar2 = true;
        do
        {
            if (fVar8 <= 0.0f)
                return;

            float length = glm::sqrt(fVar13 * fVar13 + fVar7 * fVar7 + fVar9 * fVar9);
            if (length == 0.0f)
            {
                dir_x = 1.0f;
                dir_y = 0.0f;
                dir_z = 0.0f;
            }
            else
            {
                dir_x = fVar13 / length; // Normalization
                dir_z = fVar7 / length;  //
                dir_y = fVar9 / length;  //
            }

            x += dir_x * 0.5f;
            z += dir_z * 0.5f;
            y += dir_y * 0.5f;
            fVar8 -= 0.5f;

            if (y < 126.0f && !glm::isnan(y))
                bVar2 = y < 2.0f;
                
        } while (bVar2);

        fVar9 -= 0.125f;
        dir_z = fVar8 < 0.4f ? 1.2f : 0.8f;
        dir_y = (float)(int)(-dir_z - 2.0f);
        dir_x = (float)(int)(dir_z + 2.0f);
        if (dir_y < dir_x)
        {
            BlockID block_to_place = fVar8 < 0.4f ? BlockID::light : BlockID::moon_leaf;
            x_step = dir_y;
            do
            {
                int voxel_x = (int)(x + x_step);
                z_step = dir_y;
                do
                {
                    int voxel_z = (int)(z + z_step);
                    y_step = dir_y;
                    do
                    {
                        int voxel_y = (int)(y + y_step);

                        float a = (float)voxel_x - x;
                        float b = (float)voxel_z - z;
                        float c = (float)voxel_y - y;

                        if (a*a + b*b + c*c <= dir_z * dir_z && BlockIsInChunk(voxel_x, voxel_y, voxel_z))
                        {
                            chunk[GetChunkIndex(voxel_x, voxel_y, voxel_z)] = block_to_place;
                        }

                        y_step += 1.0f;
                    } while (y_step < dir_x);

                    z_step += 1.0f;
                } while (z_step < dir_x);

                x_step += 1.0f;
            } while (x_step < dir_x);
        }
    } while (true);
}

//
// This code was reverse-engineered from Lunacraft v2.01
//
static void GenerateSpiralTreeBranch(BlockID *chunk, RNG &rng, float angle, int base_x, int base_y, int base_z)
{
    constexpr float radii[2] = {0.8f, 1.2f}; // {branch_radius, bulb_radius}

    float x = (float)base_x;
    float z = (float)base_z;
    float y = (float)base_y;

    float lateral = 0.5f * glm::sin(rng.Range(-0.2f, 0.2f) + 2.0943952f);
    float steps_remaining = rng.Range(10.0f, 25.0f);
    float delta_angle = rng.Range(0.1f, 0.6f);

    while (steps_remaining > 0.0f)
    {
        steps_remaining -= 0.5f;

        float spiral = glm::sin(angle) * 0.5f;
        float vertical = glm::cos(angle) * 0.5f;
        angle += delta_angle;

        float len = glm::sqrt(lateral*lateral + vertical*vertical + spiral*spiral);

        float dx, dy, dz;
        if (len == 0.0f)
        {
            dx = 1.0f;
            dy = 0.0f;
            dz = 0.0f;
        }
        else
        {
            dx = vertical / len;
            dy = lateral  / len;
            dz = spiral   / len;
        }

        x += dx * 0.5f;
        z += dz * 0.5f;
        y += dy * 0.5f;

        bool is_tip = steps_remaining < 0.1f;
        float radius = radii[is_tip ? 1 : 0];
        BlockID block = is_tip ? BlockID::light : BlockID::moon_wood;

        int min_offset = (int)(-radius - 2.0f);
        int max_offset = (int)( radius + 2.0f);
        for (int dxi = min_offset; dxi < max_offset; dxi++)
        {
            int xi = (int)x + dxi;

            for (int dzi = min_offset; dzi < max_offset; dzi++)
            {
                int zi = (int)z + dzi;

                for (int dyi = min_offset; dyi < max_offset; dyi++)
                {
                    int yi = (int)y + dyi;

                    float ddx = xi - x;
                    float ddz = zi - z;
                    float ddy = yi - y;

                    if (ddx*ddx + ddz*ddz + ddy*ddy <= radius*radius && BlockIsInChunk(xi, yi, zi))
                    {
                        chunk[GetChunkIndex(xi, yi, zi)] = block;
                    }
                }
            }
        }
    }
}

//
// This code was reverse-engineered from Lunacraft v2.01
//
static void GenerateColorTreeBranch(BlockID *chunk, RNG &rng, float angle, int base_x, int base_y, int base_z)
{
    constexpr BlockID BULB_BLOCKS[3] = {BlockID::sulphur_ore, BlockID::feldspar, BlockID::chalchanthite};

    BlockID bulb_block = BULB_BLOCKS[rng.Range(0, 2)];

    float x = (float)base_x;
    float z = (float)base_z;
    float y = (float)base_y;

    float steps_remaining = rng.Range(18.0f, 25.0f);
    float lateral = rng.Range(0.25f, 0.75f) * glm::sin(rng.Range(-0.2f, 0.2f) + 2.0943952f);
    float delta_lateral = rng.Range(0.03f, 0.09f);
    float delta_angle = rng.Range(-0.125f, 0.125f);

    while (true)
    {
        do
        {
            if (steps_remaining <= 0.0f)
                return;

            angle += delta_angle;

            float forward = glm::sin(angle) * 0.5f;
            float vertical = glm::cos(angle) * 0.5f;

            float len = glm::sqrt(lateral*lateral + vertical*vertical + forward*forward);

            float dx, dy, dz;
            if (len == 0.0f)
            {
                dx = 1.0f;
                dy = 0.0f;
                dz = 0.0f;
            }
            else
            {
                dx = vertical / len;
                dy = lateral  / len;
                dz = forward  / len;
            }

            x += dx * 0.5f;
            z += dz * 0.5f;
            y += dy * 0.5f;

            steps_remaining -= 0.5f;

        } while (y < 2.0f || y >= 126.0f || glm::isnan(y));

        float radius = 0.75f;
        if (steps_remaining <= 0.0f) // At the end (bulb)
            radius = rng.Range(1.5f, 2.25f);

        lateral += delta_lateral;

        int min_offset = (int)(-radius - 2);
        int max_offset = (int)( radius + 2);

        for (int dxi = min_offset; dxi < max_offset; ++dxi)
        {
            int xi = (int)x + dxi;

            for (int dzi = min_offset; dzi < max_offset; ++dzi)
            {
                int zi = (int)z + dzi;

                for (int dyi = min_offset; dyi < max_offset; ++dyi)
                {
                    int yi = (int)y + dyi;

                    float ddx = xi - x;
                    float ddz = zi - z;
                    float ddy = yi - y;

                    if (ddx*ddx + ddz*ddz + ddy*ddy <= radius * radius && BlockIsInChunk(xi, yi, zi))
                    {
                        if (steps_remaining > 0.0f)
                        {
                            chunk[GetChunkIndex(xi, yi, zi)] = BlockID::moon_bark;
                        }
                        else
                        {
                            BlockID block = (dyi == 0) ? BlockID::light : bulb_block;
                            chunk[GetChunkIndex(xi, yi, zi)] = block;
                        }
                    }
                }
            }
        }
    }
}

//
// This code was reverse-engineered from Lunacraft v2.01
//
static void GenerateVein(
    BlockID *chunk,
    RNG &rng,
    float x,
    float y,
    float z,
    float radius,
    float pitch,
    int length,
    BlockID block,
    int recursion_depth
)
{
    if (radius < 0.1f || recursion_depth <= 0)
        return;

    // Random initial horizontal angle
    float angle = rng.Range(0.0f, 2.0f * 3.1415926f);

    while (radius >= 0.1f && recursion_depth > 0)
    {
        if (length > 0)
        {
            float local_pitch = pitch;

            while (length > 0)
            {
                // Smooth random drift
                local_pitch += rng.Range(-0.05f, 0.05f);
                angle      += rng.Range(-0.05f, 0.05f);

                // Move forward
                float step = radius * 0.5f;

                x += glm::cos(angle) * step;
                z += glm::sin(angle) * step;
                y += glm::sin(local_pitch) * step;

                length -= glm::max(1, static_cast<int>(radius));

                // Stay within vertical bounds
                if (y < 2.0f || y >= 126.0f || glm::isnan(y))
                    continue;

                // Occasionally branch
                if (rng.Range(0.0f, 1.0f) < 0.035f)
                {
                    GenerateVein(
                        chunk,
                        rng,
                        x, y, z,
                        radius * 0.8f,
                        local_pitch,
                        static_cast<int>(length * 0.45f),
                        block,
                        recursion_depth - 1
                    );
                }

                // Carve / paint
                if (radius <= 1.0f)
                {
                    int xi = static_cast<int>(x);
                    int zi = static_cast<int>(z);
                    int yi = static_cast<int>(y);

                    if (BlockIsInChunk(xi, yi, zi))
                    {
                        BlockID &current = chunk[GetChunkIndex(xi, yi, zi)];

                        if (current != BlockID::air)
                            current = block;
                    }
                }
                else
                {
                    float r = radius + rng.Range(0.0f, 1.0f);
                    int rInt = static_cast<int>(r);

                    for (int dx = -rInt; dx <= rInt; ++dx)
                    for (int dz = -rInt; dz <= rInt; ++dz)
                    for (int dy =  0;    dy <= rInt; ++dy)
                    {
                        if (dx*dx + dz*dz + dy*dy > r*r)
                            continue;

                        int xi = static_cast<int>(x + dx);
                        int zi = static_cast<int>(z + dz);
                        int yi = static_cast<int>(y + dy);

                        if (!BlockIsInChunk(xi, yi, zi))
                            continue;

                        BlockID &current = chunk[GetChunkIndex(xi, yi, zi)];

                        if (current != BlockID::air)
                            current = block;
                    }
                }
            }
        }

        // Shrink and repeat
        radius *= 0.8f;
        length = static_cast<int>(length * 0.45f);
        recursion_depth--;
    }
}

static void GenerateLairShaft(BlockID *chunk, RNG &rng, int radius, int base_x, int base_z, int base_y, int length, int recursion_depth, int direction)
{
    constexpr int directions[] = {
    //  x   z   y
        0, -1,  0,
        1,  0,  0,
        0,  1,  0,
       -1,  0,  0,
        0,  0,  1,
        0,  0, -1,
    };

    constexpr int direction_map[6][4] = { // Allowed next directions for each direction
        {1, 3, 4, 5},
        {0, 2, 4, 5},
        {1, 3, 4, 5},
        {0, 2, 4, 5},
        {0, 1, 2, 3},
        {0, 1, 2, 3},
    };

    if (length <= 0 || radius <= 0 || recursion_depth <= 0)
        return;

    int middle_x = base_x;
    int middle_z = base_z;
    int middle_y = base_y;

    int dx = directions[direction * 3];     // Shaft axis
    int dz = directions[direction * 3 + 1]; //
    int dy = directions[direction * 3 + 2]; //

    int adjusted_length = direction <= 3 ? length : 2*length; // Vertical shafts are doubled

    // Generate shaft
    for (int i = 0; i < adjusted_length; i++)
    {
        for (int h_step = -radius - 1; h_step <= radius + 1; h_step++)
        {
            for (int v_step = -radius - 1; v_step <= radius + 1; v_step++)
            {
                int x = middle_x;
                int z = middle_z;
                int y = middle_y;

                // Offset based on shaft axis
                if (dy != 0)
                {
                    x += h_step;
                    z += v_step;
                }
                else if (dx != 0)
                {
                    y += v_step;
                    z += h_step;
                }
                else
                {
                    y += v_step;
                    x += h_step;
                }

                if (BlockIsInChunk(x, y ,z))
                {
                    bool on_h_edge = glm::abs(h_step) == radius + 1;
                    bool on_v_edge = glm::abs(v_step) == radius + 1;
                    BlockID &current_block = chunk[GetChunkIndex(x, y, z)];
                    if (!(on_h_edge || on_v_edge)) // Carve inside
                    {
                        current_block = BlockID::air;
                    }
                    else if ((h_step == 0 && on_v_edge) || (v_step == 0 && on_h_edge)) // Decorate sides
                    {
                        if (current_block != BlockID::air)
                        {
                            BlockID decoration_block = rng.Range(0.0f, 1.0f) > 0.01f ? BlockID::polymer : BlockID::light; // 99% polymer, 1% light
                            current_block = decoration_block;
                        }
                    }
                }
            }
        }

        middle_x += dx;
        middle_z += dz;
        middle_y += dy;
    }

    // Decorate end
    if (BlockIsInChunk(middle_x, middle_y, middle_z))
    {
        BlockID decoration_block = rng.Range(0.0f, 1.0f) > 0.01f ? BlockID::polymer : BlockID::light; // 99% polymer, 1% light
        chunk[GetChunkIndex(middle_x, middle_y, middle_z)] = decoration_block;
    }

    // Possibly spawn branching shafts
    for (int i = 0; i < radius; i++)
    {
        int middle_offset = radius + 1;
        GenerateLairShaft(
            chunk,
            rng,
            radius - 1,
            middle_x - middle_offset * dx,
            middle_z - middle_offset * dz,
            middle_y - middle_offset * dy,
            adjusted_length - 1,
            recursion_depth - 1,
            direction_map[direction][(i + rng.Range(0, 3)) % 4]
        );
    }
}
