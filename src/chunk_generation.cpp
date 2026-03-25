
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

void GenerateHeightMap(uint8_t *height_map, int chunk_x, int chunk_z, uint64_t seed, float amplitude, float frequency, float persistence, int octaves, float roughness)
{
    float frequency0 = frequency;
    float amplitude0 = amplitude;
    float divFactor = 32.0f - 4.0f * roughness;
    float height_limit;
    double offset_x = (double)(SplitMix64(seed) & 0xFFFFFFFF);
    double offset_z = (double)(SplitMix64(seed) & 0xFFFFFFFF);

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            frequency = frequency0;
            amplitude = amplitude0;
            height_limit = 0.0f;
            for (int i = 0; i < octaves; i++)
            {
                double xArg = (((x + chunk_x * CHUNK_SIZE) + offset_x) / divFactor) * frequency;
                double zArg = (((z + chunk_z * CHUNK_SIZE) + offset_z) / divFactor) * frequency;
                height_limit += SimplexNoise(xArg, zArg) * amplitude;
                frequency *= 3.0f;
                amplitude *= persistence;
            }
            height_map[z + CHUNK_SIZE * x] = (uint8_t)height_limit;
        }
    }
}

void GenerateChunk(BlockID *chunk, int chunk_x, int chunk_z, MoonSettings settings)
{
    // Seed RNG so structure placement is deterministic
    uint64_t structure_seed = settings.seed ^ ((uint64_t)chunk_x * 73856093ull) ^ ((uint64_t)chunk_z * 19349663ull);
    RNG rng{structure_seed};

    thread_local uint8_t height_maps[CHUNK_SIZE * CHUNK_SIZE * 4];

    //
    // Generate base terrain
    //

    const int ROCK_OFFSET   = 0;
    const int GRAVEL_OFFSET = 1 * CHUNK_SIZE * CHUNK_SIZE;
    const int DIRT_OFFSET   = 2 * CHUNK_SIZE * CHUNK_SIZE;
    const int SAND_OFFSET   = 3 * CHUNK_SIZE * CHUNK_SIZE;
    GenerateHeightMap(&height_maps[ROCK_OFFSET], chunk_x, chunk_z, settings.seed, 14, 0.2, 0.4, 4, settings.terrain_roughness);
    GenerateHeightMap(&height_maps[GRAVEL_OFFSET], chunk_x, chunk_z, settings.seed, 4, 0.2, 0.6, 2, settings.terrain_roughness);
    GenerateHeightMap(&height_maps[DIRT_OFFSET], chunk_x, chunk_z, settings.seed, 3, 0.2, 0.6, 3, settings.terrain_roughness);
    GenerateHeightMap(&height_maps[SAND_OFFSET], chunk_x, chunk_z, settings.seed, 3, 0.2, 0.8, 2, settings.terrain_roughness * 0.6f);

    //
    // Crater
    //

    bool spawn_crater = RNG{}.Range(0.0f, 1.0f) < 0.22f; // 22% chance (to match original average crater density)
    if (spawn_crater)
    {
        int radius = RNG{}.Range(7, 14); // was [8, 15]

        float outer_radius = (float)radius;
        float inner_radius = outer_radius * 0.75f;

        int center_x = RNG{}.Range(radius, CHUNK_SIZE - 1 - radius);
        int center_z = RNG{}.Range(radius, CHUNK_SIZE - 1 - radius);

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
                    t = u * u;  // inner bowl
                }
                else
                {
                    t = 1.0f - (dist - inner_radius) / (outer_radius - inner_radius); // rim falloff
                }

                float h = (&height_maps[DIRT_OFFSET])[z + x * CHUNK_SIZE];
                h += (outer_radius * t) / 3.5f;

                (&height_maps[DIRT_OFFSET])[z + x * CHUNK_SIZE] = (uint8_t)glm::min(h, 126.0f);
            }
        }
    }

    //
    // Layer determination
    //

    // Initial generation
    int chunk_index = 0;
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            int rock_height   = height_maps[ROCK_OFFSET   + z + CHUNK_SIZE * x];
            int gravel_height = height_maps[GRAVEL_OFFSET + z + CHUNK_SIZE * x];
            int dirt_height   = height_maps[DIRT_OFFSET   + z + CHUNK_SIZE * x];
            int sand_height   = height_maps[SAND_OFFSET   + z + CHUNK_SIZE * x];
            int y = 0;

            // Base rock
            while (y < 50 + rock_height)
            {
                chunk[chunk_index++] = BlockID::rock;
                y++;
            }

            // Base gravel
            while (y < 50 + rock_height + gravel_height)
            {
                chunk[chunk_index++] = BlockID::gravel;
                y++;
            }

            // Base dirt
            while (y < 50 + rock_height + gravel_height + dirt_height)
            {
                chunk[chunk_index++] = BlockID::dirt;
                y++;
            }

            if (50 + rock_height + gravel_height + dirt_height < GROUND_LEVEL) // Below ground level; fill rest in with sand/water
            {
                int sand_limit = glm::min(GROUND_LEVEL, 50 + rock_height + gravel_height + dirt_height + sand_height);
                while (y < sand_limit)
                {
                    chunk[chunk_index++] = BlockID::sand;
                    y++;
                }

                while (y < GROUND_LEVEL)
                {
                    chunk[chunk_index++] = BlockID::water;
                    y++;
                }
            }
            else // Above ground level; finish terrain by placing topsoil
            {
                chunk[chunk_index++] = BlockID::topsoil;
                y++;
            }

            while (y < WORLD_HEIGHT_LIMIT)
            {
                chunk[chunk_index++] = BlockID::air;
                y++;
            }
        }
    }

    //
    // Ores
    //

    int ore_spawn_chance = rng.Range(1, 10);
    if (ore_spawn_chance <= 4)
    {
        int seed_block_x = rng.Range(5, CHUNK_SIZE - 7);
        int seed_block_z = rng.Range(5, CHUNK_SIZE - 7);
        int seed_block_y = -1;
        for (int y = 63; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunk_index = GetChunkIndex(seed_block_x, y + 1, seed_block_z);
            if (chunk[chunk_index] == BlockID::air)
            {
                seed_block_y = y;
                break;
            }
        }

        int ore = rng.Range(1, 100);
        int vein_size;
        BlockID ore_id;
        if (ore <= 36) // 36%
        {
            ore_id = BlockID::magnetite;
            vein_size = rng.Range(2, 6);
        }
        else if (ore <= 60) // 24%
        {
            ore_id = BlockID::aluminum_ore;
            vein_size = rng.Range(2, 6);
        }
        else if (ore <= 78) // 18%
        {
            ore_id = BlockID::titanium_ore;
            vein_size = rng.Range(2, 6);
        }
        else if (ore <= 91) // 13%
        {
            ore_id = BlockID::gold_ore;
            vein_size = rng.Range(1, 4);
        }
        else if (ore <= 98) // 7%
        {
            ore_id = BlockID::notchium_ore;
            vein_size = rng.Range(1, 4);
        }
        else // 2%
        {
            ore_id = BlockID::blue_crystal;
            vein_size = rng.Range(1, 4);
        }

        int current_block_x = seed_block_x;
        int current_block_y = seed_block_y;
        int current_block_z = seed_block_z;
        chunk_index = GetChunkIndex(seed_block_x, seed_block_y, seed_block_z);
        chunk[chunk_index] = ore_id;
        for (int count = 0; count < vein_size; count++)
        {
            int next_direction = rng.Range(1, 5);
            if (next_direction == 1) // Forward
            {
                current_block_z++;
            }
            else if (next_direction == 2) // Backward
            {
                current_block_z--;
            }
            else if (next_direction == 3) // Right
            {
                current_block_x++;
            }
            else if (next_direction == 4) // Left
            {
                current_block_x--;
            }
            else // Down
            {
                current_block_y--;
            }

            chunk_index = GetChunkIndex(current_block_x, current_block_y, current_block_z);
            if (chunk[chunk_index] != BlockID::air)
                chunk[chunk_index] = ore_id;
        }
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
        int y = WORLD_HEIGHT_LIMIT - 1;
        while (chunk[GetChunkIndex(x, y, z)] == BlockID::air)
            y--;

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
        int y = WORLD_HEIGHT_LIMIT - 1;
        while (chunk[GetChunkIndex(x, y, z)] == BlockID::air)
            y--;

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
        int y = WORLD_HEIGHT_LIMIT - 1;
        while (chunk[GetChunkIndex(x, y, z)] == BlockID::air)
            y--;

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

        // The total height isn't determined by a single heightmap, so I have to do this...
        int y = -1;
        for (int j = 50; j < WORLD_HEIGHT_LIMIT - 1; j++) // j=50 to skip section guaranteed to be solid
        {
            if (chunk[GetChunkIndex(x, j + 1, z)] == BlockID::air)
            {
                y = j;
                break;
            }
        }

        if (y != -1)
        {
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
    }

    //
    // Astronaut lairs
    //

    bool spawn_astronaut_lair = rng.Range(1, 100) == 69; // 1% chance, each chunk
    const int lair_depth = 31;
    if (spawn_astronaut_lair)
    {
        int center_block_x = (int)(CHUNK_SIZE / 2);
        int center_block_z = (int)(CHUNK_SIZE / 2);
        int center_block_y = -1;
        for (int y = 64; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunk_index = GetChunkIndex(center_block_x, y + 1, center_block_z);
            if (chunk[chunk_index] == BlockID::air)
            {
                center_block_y = y;
                break;
            }
        }

        // Place gravel block at center of bottom so I can easily check whether a chunk contains an astronaut lair
        chunk_index = GetChunkIndex(center_block_x, 0, center_block_z);
        chunk[chunk_index] = BlockID::gravel;

        // Carve main shaft
        for (int x_offset = -3; x_offset <= 3; x_offset++)
        {
            for (int z_offset = -3; z_offset <= 3; z_offset++)
            {
                for (int y_offset = -6; y_offset <= lair_depth; y_offset++)
                {
                    chunk_index = GetChunkIndex(center_block_x + x_offset, center_block_y - y_offset, center_block_z + z_offset);
                    chunk[chunk_index] = BlockID::air;
                }
            }
        }

        // Decorate main shaft with polymer and light
        bool left_side_done = false;
        bool right_side_done = false;
        bool front_side_done = false;
        bool back_side_done = false;
        for (int y = center_block_y - lair_depth; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunk_index = GetChunkIndex(center_block_x - 4, y, center_block_z);
            if (chunk[chunk_index] != BlockID::air && !left_side_done)
            {
                chunk[chunk_index] = BlockID::polymer;
                if ((y - (center_block_y - lair_depth)) / 8.0f == 3)
                    chunk[chunk_index] = BlockID::light;
            }
            else
            {
                left_side_done = true;
            }

            chunk_index = GetChunkIndex(center_block_x + 4, y, center_block_z);
            if (chunk[chunk_index] != BlockID::air && !right_side_done)
            {
                chunk[chunk_index] = BlockID::polymer;
                if ((y - (center_block_y - lair_depth)) / 8.0f == 3)
                    chunk[chunk_index] = BlockID::light;
            }
            else
            {
                right_side_done = true;
            }

            chunk_index = GetChunkIndex(center_block_x, y, center_block_z + 4);
            if (chunk[chunk_index] != BlockID::air && !front_side_done)
            {
                chunk[chunk_index] = BlockID::polymer;
                if ((y - (center_block_y - lair_depth)) / 8.0f == 3)
                    chunk[chunk_index] = BlockID::light;
            }
            else
            {
                front_side_done = true;
            }

            chunk_index = GetChunkIndex(center_block_x, y, center_block_z - 4);
            if (chunk[chunk_index] != BlockID::air && !back_side_done)
            {
                chunk[chunk_index] = BlockID::polymer;
                if ((y - (center_block_y - lair_depth)) / 8.0f == 3)
                    chunk[chunk_index] = BlockID::light;
            }
            else
            {
                back_side_done = true;
            }

            if (left_side_done && right_side_done && front_side_done && back_side_done)
                break;
        }

        // Polymer at bottom center
        chunk_index = GetChunkIndex(center_block_x, center_block_y - lair_depth - 1, center_block_z);
        chunk[chunk_index] = BlockID::polymer;

        // Extra front shaft(s)
        if (rng.Range(1, 4) == 1)
        {
            for (int dz = 0; dz < 10; dz++)
            {
                for (int dx = -2; dx <= 2; dx++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaft_x = center_block_x + dx;
                        int shaft_y = (center_block_y - lair_depth + 3) + dy;
                        int shaft_z = (center_block_z + 4) + dz;
                        chunk_index = GetChunkIndex(shaft_x, shaft_y, shaft_z);
                        chunk[chunk_index] = BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunk_index = GetChunkIndex(center_block_x - 3, shaft_y, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Right polymer
                            chunk_index = GetChunkIndex(center_block_x + 3, shaft_y, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }

                        if (dx == 0)
                        {
                            // Top polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth + 6, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Bottom polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunk_index = GetChunkIndex(center_block_x, center_block_y - lair_depth + 3, center_block_z + 4 + 10);
            chunk[chunk_index] = BlockID::polymer;

            // Extra vertical shaft?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + dx, center_block_y - lair_depth - dy, center_block_z + 4 + 8 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + dx + dx, center_block_y - lair_depth - dy, center_block_z + 4 + 8);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x, center_block_y - lair_depth - dy, center_block_z + 4 + 8 + dz + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy, center_block_z + 12 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == -1) // left
                            {
                                chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy, center_block_z + 12 + dz - 1);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z + 12 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy, center_block_z + 12 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == -1) // right
                            {
                                chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy, center_block_z + 12 + dz - 1);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z + 12 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }

        // Extra back shaft(s)
        if (rng.Range(1, 4) == 1)
        {
            for (int dz = 0; dz < 10; dz++)
            {
                for (int dx = -2; dx <= 2; dx++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaft_x = center_block_x + dx;
                        int shaft_y = (center_block_y - lair_depth + 3) + dy;
                        int shaft_z = (center_block_z - 4) - dz;
                        chunk_index = GetChunkIndex(shaft_x, shaft_y, shaft_z);
                        chunk[chunk_index] = BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunk_index = GetChunkIndex(center_block_x - 3, shaft_y, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Right polymer
                            chunk_index = GetChunkIndex(center_block_x + 3, shaft_y, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }

                        if (dx == 0)
                        {
                            // Top polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth + 6, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Bottom polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunk_index = GetChunkIndex(center_block_x, center_block_y - lair_depth + 3, center_block_z - 4 - 10);
            chunk[chunk_index] = BlockID::polymer;

            // Extra vertical shaft?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + dx, center_block_y - lair_depth - dy, center_block_z - 12 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + dx + dx, center_block_y - lair_depth - dy, center_block_z - 12);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x, center_block_y - lair_depth - dy, center_block_z - 12 + dz + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy, center_block_z - 12 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == 1) // left
                            {
                                chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy, center_block_z - 12 + dz + 1);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z - 12 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy, center_block_z - 12 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == 1) // right
                            {
                                chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy, center_block_z - 12 + dz + 1);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z - 12 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }

        // Extra right shaft(s)
        if (rng.Range(1, 4) == 1)
        {
            for (int dx = 0; dx < 10; dx++)
            {
                for (int dz = -2; dz <= 2; dz++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaft_x = (center_block_x + 4) + dx;
                        int shaft_y = (center_block_y - lair_depth + 3) + dy;
                        int shaft_z = center_block_z + dz;
                        chunk_index = GetChunkIndex(shaft_x, shaft_y, shaft_z);
                        chunk[chunk_index] = BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunk_index = GetChunkIndex(shaft_x, shaft_y, center_block_z - 3);
                            chunk[chunk_index] = BlockID::polymer;

                            // Right polymer
                            chunk_index = GetChunkIndex(shaft_x, shaft_y, center_block_z + 3);
                            chunk[chunk_index] = BlockID::polymer;
                        }

                        if (dz == 0)
                        {
                            // Top polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth + 6, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Bottom polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunk_index = GetChunkIndex(center_block_x + 4 + 10, center_block_y - lair_depth + 3, center_block_z);
            chunk[chunk_index] = BlockID::polymer;

            // Extra vertical shaft?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + 4 + 8 + dx, center_block_y - lair_depth - dy, center_block_z + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 4 + 8 + dx + dx, center_block_y - lair_depth - dy, center_block_z);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 4 + 8, center_block_y - lair_depth - dy, center_block_z + dz + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + 12 + dx, center_block_y - lair_depth + 3 + dy, center_block_z + 3 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == -1)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 12 + dx - 1, center_block_y - lair_depth + 3 + dy, center_block_z + 3 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 12 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z + 3 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + 12 + dx, center_block_y - lair_depth + 3 + dy, center_block_z - 3 - dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == -1)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 12 + dx - 1, center_block_y - lair_depth + 3 + dy, center_block_z - 3 - dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 12 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z - 3 - dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }

        // Extra left shaft(s)
        if (rng.Range(1, 4) == 1)
        {
            for (int dx = 0; dx < 10; dx++)
            {
                for (int dz = -2; dz <= 2; dz++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaft_x = (center_block_x - 4) - dx;
                        int shaft_y = (center_block_y - lair_depth + 3) + dy;
                        int shaft_z = center_block_z + dz;
                        chunk_index = GetChunkIndex(shaft_x, shaft_y, shaft_z);
                        chunk[chunk_index] = BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunk_index = GetChunkIndex(shaft_x, shaft_y, center_block_z - 3);
                            chunk[chunk_index] = BlockID::polymer;

                            // Right polymer
                            chunk_index = GetChunkIndex(shaft_x, shaft_y, center_block_z + 3);
                            chunk[chunk_index] = BlockID::polymer;
                        }

                        if (dz == 0)
                        {
                            // Top polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth + 6, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Bottom polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunk_index = GetChunkIndex(center_block_x - 4 - 10, center_block_y - lair_depth + 3, center_block_z);
            chunk[chunk_index] = BlockID::polymer;

            // Extra vertical shaft?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x - 4 - 8 + dx, center_block_y - lair_depth - dy, center_block_z + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 4 - 8 + dx + dx, center_block_y - lair_depth - dy, center_block_z);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 4 - 8, center_block_y - lair_depth - dy, center_block_z + dz + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x - 12 + dx, center_block_y - lair_depth + 3 + dy, center_block_z + 3 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == 1)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 12 + dx + 1, center_block_y - lair_depth + 3 + dy, center_block_z + 3 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 12 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z + 3 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x - 12 + dx, center_block_y - lair_depth + 3 + dy, center_block_z - 3 - dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == 1)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 12 + dx + 1, center_block_y - lair_depth + 3 + dy, center_block_z - 3 - dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 12 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z - 3 - dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }
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
