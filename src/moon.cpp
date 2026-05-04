
#include <filesystem>
#include <fstream>
#include <thread>
#include <cstdlib>
#include <chrono>

#include <stb_image/stb_image.h>
#include <stb_image_write/stb_image_write.h>

#include "moon.h"
#include "storage.h"
#include "options.h"
#include "player.h"
#include "helpers.h"
#include "constants.h"
#include "shader.h"
#include "rng.h"
#include "dropped_item.h"
#include "sound_system.h"
#include "minilight.h"
#include "turret.h"
#include "astronaut.h"
#include "brown_mob.h"
#include "blue_mob.h"
#include "green_mob.h"
#include "input.h"
#include "fxaa.h"

Moon *Moon::current_moon_;

Moon::Moon(int moon_id, MoonSettings moon_settings)
{
    bool moon_data_exists;

    // Create moon folder (if necessary)
    std::filesystem::path moon_dir = Storage::MOONS / (std::string("moon") + std::to_string(moon_id));
    if (!std::filesystem::exists(moon_dir))
        std::filesystem::create_directory(moon_dir);

    // Use moon data from file (if it already exists)
    std::filesystem::path moon_data_path = moon_dir / "moon.dat";
    moon_data_exists = std::filesystem::exists(moon_data_path);
    if (moon_data_exists)
    {
        std::ifstream moon_data_file(moon_data_path, std::ios::binary);
        moon_data_file.read(reinterpret_cast<char *>(&moon_settings), sizeof(MoonSettings));
        moon_data_file.close();
    }
    else
    {
        std::ofstream moon_data_file(moon_data_path, std::ios::binary);
        moon_data_file.write(reinterpret_cast<char *>(&moon_settings), sizeof(MoonSettings));
        moon_data_file.close();
    }

    if (moon_data_exists)
    {
        base_fog_color_ = moon_settings.base_fog_color;
    }
    else
    {
        // Choose random fog color. This code was reverse engineered from the
        // latest version of the original game (v2.01).

        RNG rng;

        int _case = rng.Range(1, 6);
        float _rand = rng.Range(0.0f, 1.0f);
        float comp1 = (1.0 - _rand * 0.85) * 0.9;
        float comp2 = (1.0 - (1.0 - _rand) * 0.85) * 0.9;

        if (_case == 1)
            base_fog_color_ = {0.9, comp2, 0.135, 1};
        else if (_case == 2)
            base_fog_color_ = {comp1, 0.9, 0.135, 1};
        else if (_case == 3)
            base_fog_color_ = {0.135, 0.9, comp2, 1};
        else if (_case == 4)
            base_fog_color_ = {0.135, comp1, 0.9, 1};
        else if (_case == 5)
            base_fog_color_ = {comp2, 0.135, 0.9, 1};
        else
            base_fog_color_ = {0.9, 0.135, comp1, 1};
    }

    int render_distance = OptionsManager::GetOptions().render_distance;
    initial_chunk_count_ = (2*render_distance + 1) * (2*render_distance + 1);
    id_ = moon_id;
    settings_ = moon_settings;
    world_time_ = moon_settings.world_time;
    skybox_phase_ = moon_settings.skybox_phase;
    skybox_reversed_ = moon_settings.skybox_reversed;
    player_ = new Player;
    current_moon_ = this;
    chunk_manager_.Init(moon_id, moon_settings);
    entity_manager_.SetEntityCount(moon_settings.entity_count);
    entity_manager_.AddEntity(player_);
}

Moon::~Moon()
{
    entity_manager_.SaveAllEntities();
    chunk_manager_.WriteAllChunksToDisk();

    settings_.world_time = world_time_;
    settings_.skybox_phase = skybox_phase_;
    settings_.skybox_reversed = skybox_reversed_;
    settings_.base_fog_color = base_fog_color_;
    settings_.entity_count = entity_manager_.GetEntityCount();
    std::filesystem::path moon_data_path = Storage::MOONS / (std::string("moon") + std::to_string(id_)) / "moon.dat";
    std::ofstream moon_data_file(moon_data_path, std::ios::binary);
    moon_data_file.write(reinterpret_cast<char *>(&settings_), sizeof(MoonSettings));
    moon_data_file.close();

    current_moon_ = nullptr;
}

Moon *Moon::GetCurrentMoon()
{
    return current_moon_;
}

Player *Moon::GetPlayer()
{
    return player_;
}

ChunkManager &Moon::GetChunkManager()
{
    return chunk_manager_;
}

EntityManager &Moon::GetEntityManager()
{
    return entity_manager_;
}

glm::vec4 Moon::GetFogColor()
{
    float factor = glm::sin(GetSkyboxAngle());
    if (factor < 0)
        factor = 0;
    glm::vec3 fog_rgb = factor * glm::vec3(base_fog_color_);
    return {fog_rgb, base_fog_color_.a};
}

int Moon::GetID()
{
    return id_;
}

float Moon::GetLoadProgress()
{
    int loaded_chunks = chunk_manager_.GetLoadedChunkCount();
    return (float)loaded_chunks / (float)initial_chunk_count_;
}

MoonSettings Moon::GetSettings()
{
    return settings_;
}

double Moon::GetWorldTime()
{
    return world_time_;
}

// In radians
float Moon::GetSkyboxAngle()
{
    if (skybox_reversed_)
        return skybox_phase_ - LIGHT_CYCLE_OMEGA * world_time_;
    else
        return skybox_phase_ + LIGHT_CYCLE_OMEGA * world_time_;
}

// In radians
float Moon::GetSkyboxAngleClamped()
{
    return GetLightPhase() * ((2.0f * glm::pi<float>()) / LIGHT_PHASES);
}

int Moon::GetLightPhase()
{
    float skybox_angle = glm::degrees(GetSkyboxAngle());
    return ((int)skybox_angle % 360) / (360.0f / LIGHT_PHASES);
}

glm::vec3 Moon::GetSunlightDirection()
{
    float angle = glm::mod(GetSkyboxAngleClamped(), glm::pi<float>()) - glm::pi<float>();
    return {
        0.0f,
        glm::sin(angle),
        glm::cos(angle)
    };
}

void Moon::Update(double delta_time)
{
    float time_scale;

    ItemID player_item = player_->GetInventory().GetSelectedItem();
    if (player_item == ItemID::chronobooster)
    {
        time_scale = 10.0f;
    }
    else if (player_item == ItemID::chronowinder)
    {
        time_scale = 3.0f;
    }
    else
    {
        if (GetLightPhase() <= 6)
            time_scale = 1.0f; // day
        else
            time_scale = 2.0f; // night
    }

    world_time_ += time_scale * delta_time;
    accumulator_ += delta_time;

    if (player_item == ItemID::chronowinder && !skybox_reversed_)
    {
        skybox_reversed_ = true;
        skybox_phase_ += 2 * LIGHT_CYCLE_OMEGA * world_time_;
    }
    else if (player_item != ItemID::chronowinder && skybox_reversed_)
    {
        skybox_reversed_ = false;
        skybox_phase_ -= 2 * LIGHT_CYCLE_OMEGA * world_time_;
    }

    //
    // Fixed updates
    //

    if (accumulator_ >= FIXED_DELTA_TIME)
        entity_manager_.SelfUpdate();

    entity_manager_.RunPhysics(accumulator_);

    //
    // Per-frame updates
    //

    // Non-physics updates
    entity_manager_.Update(delta_time);

    // Handle chunk jobs
    chunk_manager_.HandleChunkJobs();

    // Update selection block
    UpdateSelectionBlock(delta_time);
    auto selection_position = selection_block_.GetPosition();
    auto selection_adjacent_position = selection_block_.GetAdjacentPosition();

    // Handle player modifications
    if (selection_block_.IsActive())
    {
        if (selection_block_.GetMineProgress() >= 1.0f)
        {
            BlockID block_to_drop = chunk_manager_.GetBlockAt(selection_position);
            if (block_to_drop == BlockID::topsoil)
                block_to_drop = BlockID::dirt;

            chunk_manager_.HandlePlayerModification(selection_position);

            if (block_to_drop == BlockID::minilight)
                entity_manager_.DestroyMinilightAt(selection_position);

            DroppedItem *dropped = new DroppedItem({
                .position = selection_position,
                .item = BlockIDToItemID(block_to_drop),
                .amount = 1
            });
            dropped->SetVelocity({
                RNG{}.Range(-1.0f, 1.0f),
                RNG{}.Range(0.5f, 1.0f),
                RNG{}.Range(-1.0f, 1.0f)
            });
            entity_manager_.AddEntity(dropped);

            SoundSystem::PlayAt(SoundSystem::Sound::BLOCK_BREAK, selection_position);
            selection_block_.SetMineProgress(0);
        }
        else if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) && selection_position.y < WORLD_HEIGHT_LIMIT - 1)
        {
            auto player_aabb = player_->GetAABB();
            auto player_pos = player_->GetPosition();
            auto player_displacement = glm::vec3{selection_adjacent_position} - player_pos;

            if (glm::abs(player_displacement.x) <= (player_aabb.extents.x + 0.5f)
             && glm::abs(player_displacement.z) <= (player_aabb.extents.z + 0.5f)
             && glm::abs(player_displacement.y) <= (player_aabb.extents.y + 0.5f))
                return;

            auto &inventory = player_->GetInventory();
            auto &selected = inventory.inventory[0][inventory.selected_hotbar_slot];
            if (selected.item == ItemID::turret_t1 || selected.item == ItemID::turret_t2 || selected.item == ItemID::turret_t3)
            {
                int level = selected.item == ItemID::turret_t1 ? 0
                          : selected.item == ItemID::turret_t2 ? 2
                          :                                      4;

                entity_manager_.AddEntity(new Turret({
                    .position = selection_adjacent_position,
                    .level = level
                }));

                if (!settings_.is_creative)
                {
                    selected.amount--;
                    if (selected.amount < 1)
                        selected = {ItemID::none, 0};
                }
            }
            else if (ItemIsBlock(selected.item))
            {
                BlockID block = ItemIDToBlockID(selected.item);
                if (block != BlockID::air)
                {
                    if (!settings_.is_creative)
                    {
                        selected.amount--;
                        if (selected.amount < 1)
                            selected = {ItemID::none, 0};
                    }

                    chunk_manager_.HandlePlayerModification(selection_adjacent_position, block);

                    if (block == BlockID::minilight)
                    {
                        auto normal = selection_adjacent_position - selection_position;
                        Minilight *minilight = new Minilight(MinilightData{
                            .id = 0,
                            .voxel = selection_adjacent_position,
                            .normal = normal
                        });
                        entity_manager_.AddEntity(minilight);
                    }

                    if (block == BlockID::beacon)
                    {
                        entity_manager_.AddEntity(new Astronaut({
                            .position = selection_adjacent_position + glm::ivec3{0, 20, 0},
                            .level = RNG{}.Range(0, 4),
                            .friendly = true
                        }));
                        SoundSystem::Play(SoundSystem::Sound::MEDKIT);
                    }

                    SoundSystem::PlayAt(SoundSystem::Sound::BLOCK_PLACE, selection_adjacent_position);
                }
            }
        }
    }

    // Handle brown mob explosions
    if (!brown_mob_explosions_.empty())
    {
        auto voxel = brown_mob_explosions_.back();
        brown_mob_explosions_.pop_back();
        chunk_manager_.HandleBrownMobExplosion(voxel);
    }

    // Update lighting
    int light_phase = GetLightPhase(); 
    if (light_phase != current_light_phase_)
    {
        current_light_phase_ = light_phase;
        chunk_manager_.UpdateGlobalLighting();
    }
    
    // Load new chunks around player (and unload old ones)
    if (world_time_ - last_patch_update_ >= 0.2)
    {
        chunk_manager_.AdjustChunkPatch();
        last_patch_update_ = world_time_;
    }

    // Upload any new chunks that are ready to the GPU
    chunk_manager_.UploadReadyChunks();
}

void Moon::Render(const glm::mat4 &projection)
{
    //
    // Render world
    //

    bool fxaa = OptionsManager::GetOptions().fxaa;

    if (fxaa)
        FXAA::Begin();

    Options options = OptionsManager::GetOptions();

    Camera player_camera = player_->GetCamera();
    glm::mat4 view = glm::lookAt(player_camera.position, player_camera.position + player_camera.forward, player_camera.up);
    glm::mat4 view_projection = projection * view;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Render chunks
    Shader &block_shader = ShaderManager::BLOCK_SHADER;
    block_shader.Use();

    glm::vec4 fog_color = GetFogColor();
    fog_color.a = options.show_fog ? 1 : 0;
    block_shader.SetVec4("u_fog_color", fog_color);

    block_shader.SetMat4("u_view", view);
    block_shader.SetMat4("u_proj", projection);
    block_shader.SetFloat("u_fog_start", (float)options.render_distance * 0.33f * 32.0f);
    block_shader.SetFloat("u_fog_end", (float)options.render_distance * 0.85f * 32.0f);

    Plane frustum[6];
    GetFrustumPlanes(view_projection, frustum);
    chunk_manager_.RenderChunks(frustum);

    // Render entities
    entity_manager_.RenderEntities(view, projection);

    // Render selection overlay
    selection_block_.Render(view, projection);

    //
    // Render skybox
    //

    view = glm::mat4(glm::mat3(view));
    view_projection = projection * view;
    skybox_.Update(view_projection, GetSkyboxAngle());
    skybox_.Render();

    if (fxaa)
        FXAA::End();

    //
    // Handle screenshot (not the best place, just convenient)
    //

    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) && player_->GetInventory().GetSelectedItem() == ItemID::camera)
    {
        // Read screen pixels
        auto dimensions = Viewport::GetDimensions();
        unsigned char *pixels = (unsigned char *)malloc(dimensions.x * dimensions.y * 3);
        glReadPixels(0, 0, dimensions.x, dimensions.y, GL_RGB, GL_UNSIGNED_BYTE, pixels);

        // Flip image
        for (int y = 0; y < dimensions.y / 2; ++y)
        {
            for (int x = 0; x < dimensions.x * 3; ++x)
            {
                std::swap(
                    pixels[y * dimensions.x * 3 + x],
                    pixels[(dimensions.y - 1 - y) * dimensions.x * 3 + x]
                );
            }
        }

        // Generate file name
        auto now = std::chrono::system_clock::now();
        auto days = std::chrono::floor<std::chrono::days>(now);
        std::chrono::year_month_day ymd{days};
        auto time = std::chrono::hh_mm_ss{now - days};

        std::stringstream filename;
        filename << (int)ymd.year() << "-" << (unsigned)ymd.month() << "-" << (unsigned)ymd.day();
        filename << "_" << time.hours().count() << "." << time.minutes().count() << "." << time.seconds().count() << ".png";

        // Save screenshot
        auto path = Storage::SCREENSHOTS / filename.str();
        stbi_write_png_compression_level = 4; // default is 8
        stbi_write_png(reinterpret_cast<const char *>(path.u8string().c_str()), dimensions.x, dimensions.y, 3, pixels, dimensions.x * 3);
        free(pixels);
        DisplayMessage("Screenshot saved!");

        SoundSystem::Play(SoundSystem::Sound::REWARD);
    }
}

void Moon::BrownMobExplode(glm::vec3 position)
{
    brown_mob_explosions_.push_back(GetNearestVoxel(position) - glm::ivec3{0, 2, 0});
}

void Moon::UpdateSelectionBlock(float delta_time)
{
    //
    // Update selected block
    //

    auto camera = player_->GetCamera();

    auto origin = camera.position;
    glm::ivec3 voxel = GetNearestVoxel(origin);
    glm::ivec3 last_voxel;

    glm::vec3 inv_dir = 1.0f / camera.forward;

    glm::ivec3 step = {
        camera.forward.x > 0 ? 1 : -1,
        camera.forward.y > 0 ? 1 : -1,
        camera.forward.z > 0 ? 1 : -1
    };

    glm::vec3 next_boundary = {
        voxel.x + (step.x > 0 ? 0.5f : -0.5f),
        voxel.y + (step.y > 0 ? 0.5f : -0.5f),
        voxel.z + (step.z > 0 ? 0.5f : -0.5f)
    };

    glm::vec3 t_max = (next_boundary - origin) * inv_dir;
    glm::vec3 t_delta = glm::abs(inv_dir);

    float distance = 0.0f;
    const float PLAYER_REACH = 9.0f;
    selection_block_.SetActive(false);
    while (distance < PLAYER_REACH)
    {
        if (voxel.y < 0 || voxel.y >= WORLD_HEIGHT_LIMIT)
        {
            selection_block_.SetActive(false);
            break;
        }

        auto chunk = chunk_manager_.GetChunk(VoxelToChunk(voxel));
        if (chunk)
        {
            BlockID block = chunk->GetBlocks()[GetChunkIndex(GlobalToLocalVoxel(voxel))];
            if (block != BlockID::air)
            {
                if (selection_block_.GetPosition() != voxel) // If selected block changes, reset mining
                    selection_block_.SetMineProgress(0);

                selection_block_.SetPosition(voxel);
                selection_block_.SetAdjacentPosition(last_voxel);
                selection_block_.SetActive(true);
                break;
            }
        }

        int min_comp_idx = (t_max.x < t_max.y && t_max.x < t_max.z) ? 0
                         : (t_max.y < t_max.x && t_max.y < t_max.z) ? 1
                         :                                            2;

        last_voxel = voxel;
        voxel[min_comp_idx] += step[min_comp_idx];
        distance = t_max[min_comp_idx];
        t_max[min_comp_idx] += t_delta[min_comp_idx];
    }

    //
    // Update mining level
    //

    auto player_item = player_->GetInventory().GetSelectedItem();
    bool is_mining = Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT) && !ItemIsPistol(player_item);
    float mine_progress = selection_block_.GetMineProgress();
    if (is_mining && selection_block_.GetPosition().y > 0)
    {
        // blocks/sec
        float f = player_item == ItemID::drill_t1 ? 0.7f
                : player_item == ItemID::drill_t2 ? 1.0f
                : player_item == ItemID::drill_t3 ? 1.5f
                :                                   0.08f;

        float new_progress = mine_progress + f*delta_time;
        selection_block_.SetMineProgress(new_progress);
    }
    else if (mine_progress != 0)
    {
        selection_block_.SetMineProgress(0);
    }
}

void Moon::DisplayMessage(std::string message)
{
    pending_message_ = message;
}

std::string Moon::PopPendingMessage()
{
    std::string message = pending_message_;
    pending_message_.clear();
    return message;
}

void Moon::SpawnEventEntities()
{
    auto Spawn = [this](EntityType type, int level, int count, float distance)
    {
        auto entity_count = entity_manager_.GetEntityCount();
        if (count > 0 && entity_count < 61)
        {
            auto player_pos = player_->GetPosition();
            float angle = RNG{}.Range(0.0f, 2.0f * glm::pi<float>());
            int base_x = player_pos.x + 2.0f * distance * glm::cos(angle);
            int base_z = player_pos.z + 2.0f * distance * glm::sin(angle);
            for (int i = 0; i < count; i++)
            {
                int x = base_x + RNG{}.Range(-5, 5);
                int z = base_z + RNG{}.Range(-5, 5);
                int y = 65;
                
                auto chunk = chunk_manager_.GetChunk(VoxelToChunk({x, 0, z}));
                if (!chunk) return;

                while (chunk->GetBlocks()[GetChunkIndex(x, y, z)] != BlockID::air)
                    y++;

                if (type == EntityType::ASTRONAUT)
                {
                    y += 2;
                    entity_manager_.AddEntity(new Astronaut({
                        .position = glm::vec3{x, y, z},
                        .level = level
                    }));
                }
                else if (type == EntityType::BLUE_MOB)
                {
                    y += 2;
                    entity_manager_.AddEntity(new BlueMob({
                        .position = glm::vec3{x, y, z},
                        .level = level
                    }));
                }
                else if (type == EntityType::BROWN_MOB)
                {
                    entity_manager_.AddEntity(new BrownMob({
                        .position = glm::vec3{x, y, z}
                    }));
                }
                else if (type == EntityType::GREEN_MOB)
                {
                    entity_manager_.AddEntity(new GreenMob({
                        .position = glm::vec3{x, y, z},
                        .health = RNG{}.Range(18, 42)
                    }));
                }
            }
        }
    };

    float alpha = RNG{}.Range(1.0f, 5.0f);
    float mob_chance = alpha * 0.1f;
    int level_limit = (int)alpha;

    // Astronauts
    if (RNG{}.Range(0.0f, 1.0f) < mob_chance + 0.5)
    {
        int level = RNG{}.Range(0, level_limit - 1);
        int count = RNG{}.Range(2, 4);
        float distance = RNG{}.Range(0.0f, 1.0f) * 20.0f + 5.0f;
        Spawn(EntityType::ASTRONAUT, level, count, distance);
    }

    // Blue Mob
    if (RNG{}.Range(0.0f, 1.0f) < mob_chance)
    {
        int level = RNG{}.Range(0, level_limit - 1);
        float distance = RNG{}.Range(0.0f, 1.0f) * 10.0f + 2.0f;
        Spawn(EntityType::BLUE_MOB, level, 1, distance);
    }

    mob_chance = alpha * 0.05f;

    // Brown Mob
    if (RNG{}.Range(0.0f, 1.0f) < mob_chance)
    {
        int level = RNG{}.Range(0, level_limit - 1);
        int count = RNG{}.Range(0, 2);
        float distance = RNG{}.Range(0.0f, 1.0f) * 10.0f + 5.0f;
        Spawn(EntityType::BROWN_MOB, level, count, distance);
    }

    // Green Mob
    if (RNG{}.Range(0.0f, 1.0f) < mob_chance)
    {
        int level = RNG{}.Range(0, level_limit - 1);
        int count = RNG{}.Range(0, 5);
        float distance = RNG{}.Range(0.0f, 1.0f) * 10.0f + 5.0f;
        Spawn(EntityType::GREEN_MOB, level, count, distance);
    }
}

//
// Selection block
//

SelectionBlock::SelectionBlock()
{
    float vertices[] = {
         0.505f,  0.505f, -0.505f,  1.0f, 1.0f,
         0.505f, -0.505f, -0.505f,  1.0f, 0.0f,
        -0.505f, -0.505f, -0.505f,  0.0f, 0.0f,
        -0.505f, -0.505f, -0.505f,  0.0f, 0.0f,
        -0.505f,  0.505f, -0.505f,  0.0f, 1.0f,
         0.505f,  0.505f, -0.505f,  1.0f, 1.0f,

        -0.505f, -0.505f,  0.505f,  0.0f, 0.0f,
         0.505f, -0.505f,  0.505f,  1.0f, 0.0f,
         0.505f,  0.505f,  0.505f,  1.0f, 1.0f,
         0.505f,  0.505f,  0.505f,  1.0f, 1.0f,
        -0.505f,  0.505f,  0.505f,  0.0f, 1.0f,
        -0.505f, -0.505f,  0.505f,  0.0f, 0.0f,

        -0.505f,  0.505f,  0.505f,  1.0f, 0.0f,
        -0.505f,  0.505f, -0.505f,  1.0f, 1.0f,
        -0.505f, -0.505f, -0.505f,  0.0f, 1.0f,
        -0.505f, -0.505f, -0.505f,  0.0f, 1.0f,
        -0.505f, -0.505f,  0.505f,  0.0f, 0.0f,
        -0.505f,  0.505f,  0.505f,  1.0f, 0.0f,

         0.505f, -0.505f, -0.505f,  0.0f, 1.0f,
         0.505f,  0.505f, -0.505f,  1.0f, 1.0f,
         0.505f,  0.505f,  0.505f,  1.0f, 0.0f,
         0.505f,  0.505f,  0.505f,  1.0f, 0.0f,
         0.505f, -0.505f,  0.505f,  0.0f, 0.0f,
         0.505f, -0.505f, -0.505f,  0.0f, 1.0f,

        -0.505f, -0.505f, -0.505f,  0.0f, 1.0f,
         0.505f, -0.505f, -0.505f,  1.0f, 1.0f,
         0.505f, -0.505f,  0.505f,  1.0f, 0.0f,
         0.505f, -0.505f,  0.505f,  1.0f, 0.0f,
        -0.505f, -0.505f,  0.505f,  0.0f, 0.0f,
        -0.505f, -0.505f, -0.505f,  0.0f, 1.0f,

         0.505f,  0.505f,  0.505f,  1.0f, 0.0f,
         0.505f,  0.505f, -0.505f,  1.0f, 1.0f,
        -0.505f,  0.505f, -0.505f,  0.0f, 1.0f,
        -0.505f,  0.505f, -0.505f,  0.0f, 1.0f,
        -0.505f,  0.505f,  0.505f,  0.0f, 0.0f,
         0.505f,  0.505f,  0.505f,  1.0f, 0.0f,
    };

    overlay_.SetShader(ShaderManager::SIMPLE_UNLIT_SHADER);
    overlay_.SetVertexData(vertices, sizeof(vertices) / (5 * sizeof(float)));
    overlay_.SetTexture(Storage::IMAGES / "selection_0.png");
}

void SelectionBlock::SetPosition(const glm::ivec3 &position)
{
    position_ = position;
}

glm::ivec3 SelectionBlock::GetPosition()
{
    return position_;
}

void SelectionBlock::SetAdjacentPosition(const glm::ivec3 &position)
{
    adjacent_position_ = position;
}

glm::ivec3 SelectionBlock::GetAdjacentPosition()
{
    return adjacent_position_;
}

void SelectionBlock::SetMineProgress(float progress)
{
    mine_progress_ = progress;

    int mine_level = glm::clamp((int)(progress / 0.2f), 0, 4);
    if (mine_level != mine_level_)
    {
        if (mine_level == 0)
            overlay_.SetTexture(Storage::IMAGES / "selection_0.png", GL_NEAREST);
        else if (mine_level == 1)
            overlay_.SetTexture(Storage::IMAGES / "selection_1.png", GL_NEAREST);
        else if (mine_level == 2)
            overlay_.SetTexture(Storage::IMAGES / "selection_2.png", GL_NEAREST);
        else if (mine_level == 3)
            overlay_.SetTexture(Storage::IMAGES / "selection_3.png", GL_NEAREST);
        else if (mine_level == 4)
            overlay_.SetTexture(Storage::IMAGES / "selection_4.png", GL_NEAREST);

        mine_level_ = mine_level;
    }
}

float SelectionBlock::GetMineProgress()
{
    return mine_progress_;
}

void SelectionBlock::SetActive(bool active)
{
    active_ = active;
}

bool SelectionBlock::IsActive()
{
    return active_;
}

void SelectionBlock::Render(const glm::mat4 &view, const glm::mat4 &proj)
{
    if (active_)
    {
        overlay_.Render([&](Shader *shader) {
            shader->SetMat4("u_model", glm::translate(glm::mat4{1.0f}, glm::vec3{position_}));
            shader->SetMat4("u_view", view);
            shader->SetMat4("u_proj", proj);
        });
    }
}
