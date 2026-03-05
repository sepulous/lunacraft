
#include <filesystem>
#include <fstream>
#include <thread>
#include <cstdlib>

#include <stb_image/stb_image.h>

#include "moon.h"
#include "storage.h"
#include "options.h"
#include "player.h"
#include "helpers.h"
#include "constants.h"
#include "shader.h"
#include "rng.h"

#include "input.h" // TEMP

Moon *Moon::current_moon_;

Moon::Moon(int moon_id, MoonSettings moon_settings)
{
    // Create moon folder (if necessary)
    std::filesystem::path moon_dir = Storage::MOONS / (std::string("moon") + std::to_string(moon_id));
    if (!std::filesystem::exists(moon_dir))
        std::filesystem::create_directory(moon_dir);

    // Use moon data from file (if it already exists)
    std::filesystem::path moon_data_path = moon_dir / "moon.dat";
    if (std::filesystem::exists(moon_data_path))
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

    int render_distance = OptionsManager::GetOptions().render_distance;
    initial_chunk_count_ = (2*render_distance + 1) * (2*render_distance + 1);
    id_ = moon_id;
    settings_ = moon_settings;
    world_time_ = moon_settings.world_time;
    player_ = new Player;
    current_moon_ = this;
    chunk_manager_.Init(moon_id, moon_settings);
    entity_manager_.LinkChunkManager(&chunk_manager_);
    entity_manager_.AddEntity(player_);
}

Moon::~Moon()
{
    current_moon_ = nullptr;

    // Save world time to file
    settings_.world_time = world_time_;
    std::filesystem::path moon_data_path = Storage::MOONS / (std::string("moon") + std::to_string(id_)) / "moon.dat";
    std::ofstream moon_data_file(moon_data_path, std::ios::binary);
    moon_data_file.write(reinterpret_cast<char *>(&settings_), sizeof(MoonSettings));
    moon_data_file.close();
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
    float factor = glm::sin((world_time_ + SECONDS_PER_LIGHT_PHASE) * (2 * 3.1416 / (LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE))); // The offset initializes moon on Phase 1
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

glm::vec3 Moon::GetSunlightDirection()
{
    int phase = ((int)(world_time_ + SECONDS_PER_LIGHT_PHASE) / SECONDS_PER_LIGHT_PHASE) % LIGHT_PHASES; // The offset initializes moon on Phase 1
    float main_light_angle_deg;
    if (phase <= 6) // Day
    {
        main_light_angle_deg = -180.0f + 30.0f * phase;
        return {
            0.0f,
            glm::sin(glm::radians(main_light_angle_deg)),
            glm::cos(glm::radians(main_light_angle_deg))
        };
    }
    else // Night
    {
        main_light_angle_deg = -180.0f + 60.0f * (phase - 7);
        return {
            0.0f,
            glm::sin(glm::radians(main_light_angle_deg)),
            glm::cos(glm::radians(main_light_angle_deg))
        };
    }
}

void Moon::Update(double delta_time)
{
    world_time_ += delta_time;
    accumulator_ += delta_time;

    //
    // Fixed updates
    //

    int fixed_steps = (int)(accumulator_ / FIXED_DELTA_TIME);
    accumulator_ -= fixed_steps * FIXED_DELTA_TIME;

    for (int i = 0; i < fixed_steps; i++)
    {
        entity_manager_.FixedUpdate();
        entity_manager_.PhysicsStep();
    }

    double interp = accumulator_ / FIXED_DELTA_TIME;
    entity_manager_.Interpolate(interp);

    //
    // Per-frame updates
    //

    // Non-physics updates
    entity_manager_.Update(delta_time);

    // Update selection block
    UpdateSelectionBlock();

    // Handle chunk jobs
    chunk_manager_.HandleChunkJobs();

    // Handle player modifications
    if (selection_block_.IsActive() && player_->IsInControl())
    {
        if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            chunk_manager_.HandlePlayerModification(selection_block_.GetPosition());
        }
        else if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
        {
            auto &inventory = player_->GetInventory();
            auto &selected = inventory.inventory[0][inventory.selected_hotbar_slot];
            if (ItemIsBlock(selected.item))
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

                    chunk_manager_.HandlePlayerModification(selection_block_.GetAdjacentPosition(), block);
                }
            }
        }
    }

    // Update lighting
    int light_phase = ((int)(world_time_ + SECONDS_PER_LIGHT_PHASE) / SECONDS_PER_LIGHT_PHASE) % LIGHT_PHASES; // The offset initializes moon on Phase 1
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

    Options options = OptionsManager::GetOptions();

    Camera player_camera = player_->GetCamera();
    glm::mat4 view = glm::lookAt(player_camera.position, player_camera.position + player_camera.forward, player_camera.up);
    glm::mat4 view_projection = projection * view;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Shader &block_shader = ShaderManager::BLOCK_SHADER;
    block_shader.Use();
    block_shader.SetMat4("u_view_projection", view_projection);
    block_shader.SetVec3("u_ws_camera_position", player_camera.position);
    glm::vec4 fog_color = GetFogColor();
    if (!options.show_fog)
        fog_color.a = 0;
    block_shader.SetVec4("u_fog_color", fog_color);
    block_shader.SetFloat("u_fog_distance", options.render_distance * (CHUNK_SIZE / 1.5f));
    Plane frustum[6];
    GetFrustumPlanes(view_projection, frustum);
    chunk_manager_.RenderChunks(frustum);

    selection_block_.Render(view_projection);

    //glDisable(GL_DEPTH_TEST);
    player_->RenderArm(view_projection);
    //glEnable(GL_DEPTH_TEST);

    //
    // Render skybox
    //

    view = glm::mat4(glm::mat3(view));
    view_projection = projection * view;
    float skybox_angle = (world_time_ + SECONDS_PER_LIGHT_PHASE) * (2 * 3.1416f / (LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE)); // The offset initializes moon on Phase 1
    skybox_.Update(view_projection, skybox_angle);
    skybox_.Render();
}

void Moon::UpdateSelectionBlock()
{
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
        BlockID block = chunk_manager_.GetChunk(VoxelToChunk(voxel))->GetBlocks()[GetChunkIndex(GlobalToLocalVoxel(voxel))];
        if (block != BlockID::air)
        {
            selection_block_.SetPosition(voxel);
            selection_block_.SetAdjacentPosition(last_voxel);
            selection_block_.SetActive(true);
            break;
        }

        int min_comp_idx = (t_max.x < t_max.y && t_max.x < t_max.z) ? 0
                         : (t_max.y < t_max.x && t_max.y < t_max.z) ? 1
                         :                                            2;

        last_voxel = voxel;
        voxel[min_comp_idx] += step[min_comp_idx];
        distance = t_max[min_comp_idx];
        t_max[min_comp_idx] += t_delta[min_comp_idx];
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

    mesh_.SetShader(ShaderManager::SIMPLE_UNLIT_SHADER);
    mesh_.SetVertexData(vertices, sizeof(vertices) / (5 * sizeof(float)));
    mesh_.SetTexture(Storage::IMAGES / "selection_block.png");
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

void SelectionBlock::SetActive(bool active)
{
    active_ = active;
}

bool SelectionBlock::IsActive()
{
    return active_;
}

void SelectionBlock::Render(const glm::mat4 &view_projection)
{
    if (active_)
    {
        auto mvp_matrix = glm::translate(view_projection, glm::vec3{position_});
        mesh_.Render([&](Shader *shader) {
            shader->SetMat4("u_mvp_matrix", mvp_matrix);
        });
    }
}
