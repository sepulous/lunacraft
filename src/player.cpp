
#include "player.h"
#include "constants.h"
#include "sound_system.h"
#include "input.h"
#include "storage.h"

Player::Player()
{
    _position = glm::vec3(CHUNK_SIZE / 2.0f, 114.0f + 0.5f + 0.9f, CHUNK_SIZE / 2.0f);
    _prev_position = _position;
    _next_position = _position;
    _velocity = glm::vec3(0);
    _aabb.center = _position;
    _aabb.extents = glm::vec3(0.4f, 0.9f, 0.4f);
    _camera.position = _position + glm::vec3(0, 0.9f, 0);

    // Arm mesh
    float arm_vertices[] = {
        // Side
        0.25f, -0.25f, 0.0f, 0.0f, 0.0f,
        0.25f,  0.25f, 0.0f, 0.5f, 0.0f,
        0.25f,  0.25f, 1.5f, 0.5f, 1.0f,
        0.25f,  0.25f, 1.5f, 0.5f, 1.0f,
        0.25f, -0.25f, 1.5f, 0.0f, 1.0f,
        0.25f, -0.25f, 0.0f, 0.0f, 0.0f,

        // Top
         0.25f, 0.25f, 1.5f, 0.5f, 1.0f,
         0.25f, 0.25f, 0.0f, 0.5f, 0.0f,
        -0.25f, 0.25f, 0.0f, 1.0f, 0.0f,
        -0.25f, 0.25f, 0.0f, 1.0f, 0.0f,
        -0.25f, 0.25f, 1.5f, 1.0f, 1.0f,
         0.25f, 0.25f, 1.5f, 0.5f, 1.0f,
    };
    _arm_mesh.SetShader(ShaderManager::SIMPLE_UNLIT_SHADER);
    _arm_mesh.SetVertexData(arm_vertices, sizeof(arm_vertices) / (5 * sizeof(float)));
    _arm_mesh.SetTexture(Storage::IMAGES / "player_arm.png");

    // Drill base mesh
    float drill_uv_cutoff = 8.0f / 32.0f;
    float drill_base_vertices[] = {
        // Back
        -1.0f, -1.0f, -0.0f, 0.0f, drill_uv_cutoff,
         1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f, 1.0f, drill_uv_cutoff,
        -1.0f, -1.0f, -1.0f, 0.0f, drill_uv_cutoff,

        // Side
        -1.0f, -1.0f, -1.0f, 1.0f, drill_uv_cutoff,
        -1.0f,  1.0f, -1.0f, 0.0f, drill_uv_cutoff,
        -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 1.0f, drill_uv_cutoff,

        // Top
         1.0f, 1.0f, -1.0f, 0.0f, drill_uv_cutoff,
         1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 1.0f, drill_uv_cutoff,
         1.0f, 1.0f, -1.0f, 0.0f, drill_uv_cutoff,
    };
    _drill_base_mesh.SetShader(ShaderManager::SIMPLE_UNLIT_SHADER);
    _drill_base_mesh.SetVertexData(drill_base_vertices, sizeof(drill_base_vertices) / (5 * sizeof(float)));
    _drill_base_mesh.SetTexture(Storage::IMAGES / "player_drill.png");

    // Drill bit mesh
    float drill_bit_vertices[] = {
        // Side
        -1.0f, -1.0f, -6.0f, 0.0f, drill_uv_cutoff,
        -1.0f,  1.0f, -6.0f, 1.0f, drill_uv_cutoff,
        -1.0f,  1.0f,  6.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  6.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  6.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -6.0f, 0.0f, drill_uv_cutoff,

        // Top
        -1.0f, 1.0f, -6.0f, 0.0f, drill_uv_cutoff,
         1.0f, 1.0f, -6.0f, 1.0f, drill_uv_cutoff,
         1.0f, 1.0f,  6.0f, 1.0f, 1.0f,
         1.0f, 1.0f,  6.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,  6.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, -6.0f, 0.0f, drill_uv_cutoff,
    };
    _drill_bit_mesh.SetShader(ShaderManager::SIMPLE_UNLIT_SHADER);
    _drill_bit_mesh.SetVertexData(drill_bit_vertices, sizeof(drill_bit_vertices) / (5 * sizeof(float)));
    _drill_bit_mesh.SetTexture(Storage::IMAGES / "player_drill.png");

    // Sprite mesh
    float sprite_vertices[] = {
        -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         1.0f, 2.0f, 0.0f, 1.0f, 1.0f,
         1.0f, 2.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 2.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    };
    _sprite_mesh.SetShader(ShaderManager::SIMPLE_UNLIT_SHADER);
    _sprite_mesh.SetVertexData(sprite_vertices, sizeof(sprite_vertices) / (5 * sizeof(float)));
}

void Player::Update(float delta_time)
{
    _input_direction = glm::vec3(0);

    auto forward = GetForward();
    auto right = GetRight();
    if (Input::IsKeyHeld(GLFW_KEY_W))
        _input_direction += forward;
    if (Input::IsKeyHeld(GLFW_KEY_S))
        _input_direction -= forward;
    if (Input::IsKeyHeld(GLFW_KEY_A))
        _input_direction -= right;
    if (Input::IsKeyHeld(GLFW_KEY_D))
        _input_direction += right;
    if (Input::IsKeyHeld(GLFW_KEY_SPACE) && _is_grounded)
        _is_jumping = true;

    if (glm::length(_input_direction) > 0)
        _input_direction = glm::normalize(_input_direction);

    // Regen health
    if (_suit_status > 0 && _time_since_last_health_update > 0.5f)
    {
        _health = glm::clamp(_health + 1, 0, 100);
        _time_since_last_health_update = 0;
    }
    else
    {
        _time_since_last_health_update += delta_time;
    }

    // Regen suit status
    if (CanRegenSuit() && _time_since_last_suit_update > GetSuitRegenInterval())
    {
        _suit_status = glm::clamp(_suit_status + 1, 0, 100);
        _time_since_last_suit_update = 0;
    }
    else
    {
        _time_since_last_suit_update += delta_time;
    }

    // Decide whether we're flying
    if (Input::IsKeyHeld(GLFW_KEY_SPACE) && _jetpack_energy > 0)
    {
        _time_since_started_flying += delta_time;
        if (_time_since_started_flying > 0.5f)
            _is_flying = true;
    }
    else if (Input::IsKeyReleased(GLFW_KEY_SPACE) || _jetpack_energy < 1)
    {
        _time_since_started_flying = 0;
        _is_flying = false;
    }

    // Deplete/regen jetpack energy
    if (_is_flying && _time_since_last_jetpack_update > 0.055f)
    {
        _jetpack_energy = glm::clamp(_jetpack_energy - 1, 0, GetMaxJetpackEnergy());
        _time_since_last_jetpack_update = 0;
    }
    else if (!_is_flying && _time_since_last_jetpack_update > 1.0f)
    {
        _jetpack_energy = glm::clamp(_jetpack_energy + 1, 0, GetMaxJetpackEnergy());
        _time_since_last_jetpack_update = 0;
    }
    else
    {
        _time_since_last_jetpack_update += delta_time;
    }

    _camera.position = _position + glm::vec3(0, 0.9f, 0);
}

void Player::FixedUpdate()
{
    //
    // Horizontal movement
    // ==============================================================================
    //
    // To simulate natural movement, I have a linear drag force acting on the player:
    //
    //         dv/dt = αI - βv    where I is the normalized input direction
    //
    // Since α/β is the terminal speed of the player when only moving along one axis,
    // we must have this ratio equal to the maximum move speed we want.
    //
    // The friction and maximum speed depend on whether the player is on ice.
    //

    float max_move_speed = GetMaxMoveSpeed();
    float friction = IsOnIce() ? 4.0f : 10.0f;
    float alpha = friction * 8.0f;
    float beta = alpha / max_move_speed;

    _velocity.x += (alpha * _input_direction.x - beta * _velocity.x) * FIXED_DELTA_TIME;
    if (glm::abs(_velocity.x) < 0.01f)
        _velocity.x = 0;

    _velocity.z += (alpha * _input_direction.z - beta * _velocity.z) * FIXED_DELTA_TIME;
    if (glm::abs(_velocity.z) < 0.01f)
        _velocity.z = 0;

    //
    // Jumping/flying
    //

    if (_is_jumping && _is_grounded)
    {
        _velocity.y = 3.5f;
        _is_jumping = false;
    }

    if (_is_flying)
    {
        _velocity.y = (4.0f * FIXED_DELTA_TIME) + 6.0f; // First term counteracts gravity
    }

    // Play land sound when landing
    if (!_was_grounded && _is_grounded)
    {
        if (_fall_time >= 2.0f)
            SoundSystem::Play(SoundSystem::Sound::LAND);
        _fall_time = 0;
    }
    else if (!_was_grounded)
    {
        _fall_time += FIXED_DELTA_TIME;
    }
    _was_grounded = _is_grounded;
}

void Player::UpdateCamera()
{
    auto mouse_delta = Input::GetMouseDelta();
    _camera.yaw += mouse_delta.x * _camera.sensitivity;
    _camera.pitch += mouse_delta.y * _camera.sensitivity;
    _camera.pitch = glm::clamp(_camera.pitch, -89.8f, 89.8f);

    glm::vec3 direction;
    direction.x = cos(glm::radians(_camera.yaw)) * cos(glm::radians(_camera.pitch));
    direction.y = sin(glm::radians(_camera.pitch));
    direction.z = sin(glm::radians(_camera.yaw)) * cos(glm::radians(_camera.pitch));
    _camera.forward = glm::normalize(direction);
    _camera.right = glm::normalize(glm::cross(_camera.forward, _camera.up));
}

PlayerData Player::GetPlayerData()
{
    PlayerData player_data;
    player_data.health = _health;
    player_data.suit_status = _suit_status;
    player_data.jetpack_energy = _jetpack_energy;
    player_data.position = _position;
    player_data.camera_rotation = GetCameraRotation();
    player_data.inventory = _inventory;
    return player_data;
}

Camera &Player::GetCamera()
{
    return _camera;
}

void Player::SetHealth(int health)
{
    _health = health;
}

int Player::GetHealth()
{
    return _health;
}

void Player::SetSuitStatus(int suit_status)
{
    _suit_status = suit_status;
}

int Player::GetSuitStatus()
{
    return _suit_status;
}

Inventory &Player::GetInventory()
{
    return _inventory;
}

void Player::SetInventory(Inventory inventory)
{
    _inventory = inventory;
}

void Player::SetCameraRotation(glm::vec2 rotation)
{
    _camera.pitch = rotation.x;
    _camera.yaw = rotation.y;
}

glm::vec2 Player::GetCameraRotation()
{
    return {_camera.pitch, _camera.yaw};
}

void Player::SetCameraSensitivity(float sensitivity)
{
    _camera.SetSensitivity(sensitivity);
}

glm::vec3 Player::GetForward()
{
    return {
        glm::cos(glm::radians(_camera.yaw)),
        0,
        glm::sin(glm::radians(_camera.yaw))
    };
}

glm::vec3 Player::GetRight()
{
    return glm::cross(GetForward(), glm::vec3{0, 1, 0});
}

void Player::RenderArm(const glm::mat4 &vp_matrix)
{
    auto inv_view = glm::inverse(_camera.GetViewMatrix()); // To do this in camera space

    // Arm
    auto arm_model_matrix = glm::mat4(1.0);
    arm_model_matrix = glm::translate(arm_model_matrix, {0.48f, -0.35f, -0.35f});
    arm_model_matrix = glm::scale(arm_model_matrix, {-0.45f, 0.45f, -0.45f});
    _arm_mesh.Render(vp_matrix * inv_view * arm_model_matrix);

    ItemID selected_item = _inventory.inventory[0][_inventory.selected_hotbar_slot].item;
    if (selected_item != ItemID::none)
    {
        if (selected_item == ItemID::slug_pistol_t1 || selected_item == ItemID::slug_pistol_t2 || selected_item == ItemID::slug_pistol_t3)
        {
            // _pistol_base_mesh.Render(render_matrix);
            // _pistol_slide_mesh.Render(render_matrix);
        }
        else if (selected_item == ItemID::drill_t1 || selected_item == ItemID::drill_t2 || selected_item == ItemID::drill_t3)
        {
            // Drill base
            auto drill_base_model_matrix = glm::mat4(1.0);
            drill_base_model_matrix = glm::translate(drill_base_model_matrix, {0.5f, -0.35f, -1.0f});
            drill_base_model_matrix = glm::scale(drill_base_model_matrix, {0.15f, 0.15f, -0.15f});
            _drill_base_mesh.Render(vp_matrix * inv_view * drill_base_model_matrix);

            // Drill bit
            auto drill_bit_model_matrix = glm::mat4(1.0);
            drill_bit_model_matrix = glm::translate(drill_bit_model_matrix, {0.5f, -0.35f, -1.5f});
            drill_bit_model_matrix = glm::scale(drill_bit_model_matrix, {0.05f, 0.05f, -0.05f});
            _drill_bit_mesh.Render(vp_matrix * inv_view * drill_bit_model_matrix);

            _last_held_item = ItemID::drill_t1;
        }
        else if (ItemIsSprite(selected_item))
        {
            auto sprite_model_matrix = glm::mat4(1.0);
            sprite_model_matrix = glm::translate(sprite_model_matrix, {0.362f, -0.335f, -0.95f});
            sprite_model_matrix = glm::scale(sprite_model_matrix, {0.14f, 0.14f, -0.14f});

            if (_last_held_item != selected_item)
            {
                _sprite_mesh.SetTexture(Storage::IMAGES / "items" / GetItemFile(selected_item));
                _last_held_item = selected_item;
            }
            _sprite_mesh.Render(vp_matrix * inv_view * sprite_model_matrix);
        }
        else
        {
            // _block_mesh.Render(render_matrix);
        }
    }
}

// In blocks/second
float Player::GetMaxMoveSpeed()
{
    float max_speed;

    auto battery_item = _inventory.spacesuit[1].item;
    if (battery_item == ItemID::battery)
        max_speed = 4.74f;
    else if (battery_item == ItemID::power_crystal)
        max_speed = 6.4f;
    else if (battery_item == ItemID::energy_orb)
        max_speed = 8.5f;
    else
        max_speed = 2.82f;

    if (IsOnIce())
        max_speed *= 1.2f;

    return max_speed;
}

bool Player::CanRegenSuit()
{
    auto battery_item = _inventory.spacesuit[1].item;
    return battery_item == ItemID::battery || battery_item == ItemID::power_crystal || battery_item == ItemID::energy_orb;
}

// In seconds
float Player::GetSuitRegenInterval()
{
    auto battery_item = _inventory.spacesuit[1].item;
    if (battery_item == ItemID::battery)
        return 3.3f;
    else if (battery_item == ItemID::power_crystal)
        return 1.35f;
    else if (battery_item == ItemID::energy_orb)
        return 0.65f;
    else
        return std::numeric_limits<float>::max();
}

void Player::SetJetpackEnergy(int energy)
{
    _jetpack_energy = energy;
}

int Player::GetJetpackEnergy()
{
    return _jetpack_energy;
}

int Player::GetMaxJetpackEnergy()
{
    auto jetpack_item = _inventory.spacesuit[0].item;
    if (jetpack_item == ItemID::jetpack_t1)
        return 20;
    else if (jetpack_item == ItemID::jetpack_t2)
        return 40;
    else if (jetpack_item == ItemID::jetpack_t3)
        return 75;
    else
        return 0;
}
