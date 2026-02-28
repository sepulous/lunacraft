
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

Moon *Moon::_current_moon;

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

    // Choose random fog color (this code was reverse engineered from an
    // unknown version of the original game; definitely at least v1.91)

    RNG rng;

    int _case = rng.Range(1, 6);
    float _rand = rng.Range(0.0f, 1.0f);
    float comp1 = (1.0 - _rand * 0.85) * 0.9;
    float comp2 = (1.0 - (1.0 - _rand) * 0.85) * 0.9;

    if (_case == 1)
        _base_fog_color = {0.9, comp2, 0.135, 1};
    else if (_case == 2)
        _base_fog_color = {comp1, 0.9, 0.135, 1};
    else if (_case == 3)
        _base_fog_color = {0.135, 0.9, comp2, 1};
    else if (_case == 4)
        _base_fog_color = {0.135, comp1, 0.9, 1};
    else if (_case == 5)
        _base_fog_color = {comp2, 0.135, 0.9, 1};
    else
        _base_fog_color = {0.9, 0.135, comp1, 1};

    int render_distance = OptionsManager::GetOptions().render_distance;
    _initial_chunk_count = (2*render_distance + 1) * (2*render_distance + 1);
    _id = moon_id;
    _settings = moon_settings;
    _world_time = moon_settings.world_time;
    _player = new Player;
    _current_moon = this;
    _chunk_manager.Init(moon_id, moon_settings);
    _entity_manager.LinkChunkManager(&_chunk_manager);
    _entity_manager.AddEntity(_player);
}

Moon::~Moon()
{
    _current_moon = nullptr;

    // Save world time to file
    _settings.world_time = _world_time;
    std::filesystem::path moon_data_path = Storage::MOONS / (std::string("moon") + std::to_string(_id)) / "moon.dat";
    std::ofstream moon_data_file(moon_data_path, std::ios::binary);
    moon_data_file.write(reinterpret_cast<char *>(&_settings), sizeof(MoonSettings));
    moon_data_file.close();
}

Moon *Moon::GetCurrentMoon()
{
    return _current_moon;
}

Player *Moon::GetPlayer()
{
    return _player;
}

ChunkManager &Moon::GetChunkManager()
{
    return _chunk_manager;
}

EntityManager &Moon::GetEntityManager()
{
    return _entity_manager;
}

glm::vec4 Moon::GetFogColor()
{
    float factor = glm::sin((_world_time + SECONDS_PER_LIGHT_PHASE) * (2 * 3.1416 / (LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE))); // The offset initializes moon on Phase 1
    if (factor < 0)
        factor = 0;
    glm::vec3 fog_rgb = factor * glm::vec3(_base_fog_color);
    return {fog_rgb, _base_fog_color.a};
}

int Moon::GetID()
{
    return _id;
}

float Moon::GetLoadProgress()
{
    int loaded_chunks = _chunk_manager.GetLoadedChunkCount();
    return (float)loaded_chunks / (float)_initial_chunk_count;
}

MoonSettings Moon::GetSettings()
{
    return _settings;
}

double Moon::GetWorldTime()
{
    return _world_time;
}

glm::vec3 Moon::GetSunlightDirection()
{
    int phase = ((int)(_world_time + SECONDS_PER_LIGHT_PHASE) / SECONDS_PER_LIGHT_PHASE) % LIGHT_PHASES; // The offset initializes moon on Phase 1
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
    _world_time += delta_time;
    _accumulator += delta_time;

    glm::ivec3 old_player_chunk = VoxelToChunk(GetNearestVoxel(_player->GetPosition())); // Save before allowing player to move

    // Fixed updates
    if (_accumulator >= FIXED_DELTA_TIME)
        _entity_manager.FixedUpdate();

    // Physics
    int physics_steps = 0;
    if (_accumulator >= FIXED_DELTA_TIME)
        physics_steps = (int)((_accumulator - FIXED_DELTA_TIME) / FIXED_DELTA_TIME) + 1;
    _accumulator -= physics_steps * FIXED_DELTA_TIME;
    _entity_manager.RunPhysics(physics_steps, _accumulator / FIXED_DELTA_TIME);

    // Non-physics updates
    _entity_manager.Update(delta_time);

    // Update selection block
    UpdateSelectionBlock();

    // Handle player modifications
    if (_selection_block.IsActive())
    {
        if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            _chunk_manager.HandlePlayerModification(_selection_block.GetPosition());
        }
        else if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
        {
            auto &inventory = _player->GetInventory();
            auto &selected = inventory.inventory[0][inventory.selected_hotbar_slot];
            if (ItemIsBlock(selected.item))
            {
                BlockID block = ItemIDToBlockID(selected.item);
                if (block != BlockID::air)
                {
                    if (!_settings.is_creative)
                    {
                        selected.amount--;
                        if (selected.amount < 1)
                            selected = {ItemID::none, 0};
                    }

                    _chunk_manager.HandlePlayerModification(_selection_block.GetAdjacentPosition(), block);
                }
            }
        }
    }

    // Update lighting
    int light_phase = ((int)(_world_time + SECONDS_PER_LIGHT_PHASE) / SECONDS_PER_LIGHT_PHASE) % LIGHT_PHASES; // The offset initializes moon on Phase 1
    if (light_phase != _current_light_phase)
    {
        _current_light_phase = light_phase;
        _chunk_manager.UpdateGlobalLighting();
    }
    
    // Load new chunks around player (and unload old ones)
    if (_world_time - _last_patch_update >= 0.2)
    {
        _chunk_manager.AdjustChunkPatch();
        _last_patch_update = _world_time;
    }

    // Upload any new chunks that are ready to the GPU
    _chunk_manager.UploadReadyChunks();
}

void Moon::Render(const glm::mat4 &projection)
{
    //
    // Render world
    //

    Options options = OptionsManager::GetOptions();

    Camera player_camera = _player->GetCamera();
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
    _chunk_manager.RenderChunks(frustum);

    _selection_block.Render(view_projection);

    //glDisable(GL_DEPTH_TEST);
    _player->RenderArm(view_projection);
    //glEnable(GL_DEPTH_TEST);

    //
    // Render skybox
    //

    view = glm::mat4(glm::mat3(view));
    view_projection = projection * view;
    float skybox_angle = (_world_time + SECONDS_PER_LIGHT_PHASE) * (2 * 3.1416f / (LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE)); // The offset initializes moon on Phase 1
    _skybox.Update(view_projection, skybox_angle);
    _skybox.Render();
}

void Moon::UpdateSelectionBlock()
{
    auto camera = _player->GetCamera();

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
    _selection_block.SetActive(false);
    while (distance < PLAYER_REACH)
    {
        BlockID block = _chunk_manager.GetChunk(VoxelToChunk(voxel))->GetBlocks()[GetChunkIndex(GlobalToLocalVoxel(voxel))];
        if (BlockIsOpaque(block))
        {
            _selection_block.SetPosition(voxel);
            _selection_block.SetAdjacentPosition(last_voxel);
            _selection_block.SetActive(true);
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

    _mesh.SetShader(ShaderManager::SIMPLE_UNLIT_SHADER);
    _mesh.SetVertexData(vertices, sizeof(vertices) / (5 * sizeof(float)));
    _mesh.SetTexture(Storage::IMAGES / "selection_block.png");
}

void SelectionBlock::SetPosition(const glm::ivec3 &position)
{
    _position = position;
}

glm::ivec3 SelectionBlock::GetPosition()
{
    return _position;
}

void SelectionBlock::SetAdjacentPosition(const glm::ivec3 &position)
{
    _adjacent_position = position;
}

glm::ivec3 SelectionBlock::GetAdjacentPosition()
{
    return _adjacent_position;
}

void SelectionBlock::SetActive(bool active)
{
    _active = active;
}

bool SelectionBlock::IsActive()
{
    return _active;
}

void SelectionBlock::Render(const glm::mat4 &view_projection)
{
    if (_active)
    {
        auto mvp_matrix = glm::translate(view_projection, glm::vec3{_position});
        _mesh.Render([&](Shader *shader) {
            shader->SetMat4("u_mvp_matrix", mvp_matrix);
        });
    }
}
