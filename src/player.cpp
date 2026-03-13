
#include "player.h"
#include "constants.h"
#include "input.h"
#include "storage.h"
#include "block.h"
#include "helpers.h"
#include "rng.h"
#include "slug.h"
#include "moon.h"

Player::Player()
{
    type_ = EntityType::PLAYER;
    position_ = glm::vec3(CHUNK_SIZE / 2.0f, 114.0f + 0.5f + 0.9f, CHUNK_SIZE / 2.0f);
    prev_position_ = position_;
    next_position_ = position_;
    velocity_ = glm::vec3(0);
    aabb_.center = position_;
    aabb_.extents = glm::vec3(0.4f, 0.9f, 0.4f);
    camera_.position = position_ + glm::vec3(0, 0.9f, 0);
    health_ = 100;

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
    arm_mesh_.SetShader(ShaderManager::MOB_SHADER);
    arm_mesh_.SetVertexData(arm_vertices, sizeof(arm_vertices) / (5 * sizeof(float)));
    arm_mesh_.SetTexture(Storage::IMAGES / "entities" / "player_arm.png");

    // Drill base mesh
    float drill_uv_cutoff = 8.0f / 32.0f;
    float drill_base_vertices[] = {
        // Back
        -1.001f, -1.0f,   -1.0f - 0.001f, 0.0f, drill_uv_cutoff,
         1.0f,   -1.0f,   -1.0f - 0.001f, 0.0f, 0.0f,
         1.0f,    1.001f, -1.0f - 0.001f, 1.0f, 0.0f,
         1.0f,    1.001f, -1.0f - 0.001f, 1.0f, 0.0f,
        -1.001f,  1.001f, -1.0f - 0.001f, 1.0f, drill_uv_cutoff,
        -1.001f, -1.0f,   -1.0f - 0.001f, 0.0f, drill_uv_cutoff,

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
    drill_base_mesh_.SetShader(ShaderManager::MOB_SHADER);
    drill_base_mesh_.SetVertexData(drill_base_vertices, sizeof(drill_base_vertices) / (5 * sizeof(float)));
    drill_base_mesh_.SetTexture(Storage::IMAGES / "entities" / "player_drill.png");

    // Drill bit mesh
    float drill_bit_vertices[] = {
        // Left
        -1.0f, -1.0f, -6.0f, 0.0f, drill_uv_cutoff,
        -1.0f,  1.0f, -6.0f, 1.0f, drill_uv_cutoff,
        -1.0f,  1.0f,  6.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  6.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  6.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -6.0f, 0.0f, drill_uv_cutoff,

        // Right
         1.0f,  1.0f,  6.0f, 1.0f, 1.0f,
         1.0f,  1.0f, -6.0f, 1.0f, drill_uv_cutoff,
         1.0f, -1.0f, -6.0f, 0.0f, drill_uv_cutoff,
         1.0f, -1.0f, -6.0f, 0.0f, drill_uv_cutoff,
         1.0f, -1.0f,  6.0f, 0.0f, 1.0f,
         1.0f,  1.0f,  6.0f, 1.0f, 1.0f,

        // Top
        -1.0f, 1.0f, -6.0f, 0.0f, drill_uv_cutoff,
         1.0f, 1.0f, -6.0f, 1.0f, drill_uv_cutoff,
         1.0f, 1.0f,  6.0f, 1.0f, 1.0f,
         1.0f, 1.0f,  6.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,  6.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, -6.0f, 0.0f, drill_uv_cutoff,

        // Bottom
         1.0f, -1.0f,  6.0f, 1.0f, 1.0f,
         1.0f, -1.0f, -6.0f, 1.0f, drill_uv_cutoff,
        -1.0f, -1.0f, -6.0f, 0.0f, drill_uv_cutoff,
        -1.0f, -1.0f, -6.0f, 0.0f, drill_uv_cutoff,
        -1.0f, -1.0f,  6.0f, 0.0f, 1.0f,
         1.0f, -1.0f,  6.0f, 1.0f, 1.0f,
    };
    drill_bit_mesh_.SetShader(ShaderManager::MOB_SHADER);
    drill_bit_mesh_.SetVertexData(drill_bit_vertices, sizeof(drill_bit_vertices) / (5 * sizeof(float)));
    drill_bit_mesh_.SetTexture(Storage::IMAGES / "entities" / "player_drill.png");

    // Pistol base mesh
    float pistol_base_vertices[] = {
        // Side
        -1.0f, -1.0f,  4.0f,  0.0f,            0.0f,
        -1.0f, -1.0f, -4.0f, (12.0f / 32.0f),  0.0f,
        -1.0f,  1.0f, -4.0f, (12.0f / 32.0f), (4.0f / 16.0f),
        -1.0f,  1.0f, -4.0f, (12.0f / 32.0f), (4.0f / 16.0f),
        -1.0f,  1.0f,  4.0f,  0.0f,           (4.0f / 16.0f),
        -1.0f, -1.0f,  4.0f,  0.0f,            0.0f,

        // Top
        -1.0f, 1.0f, -4.0f,  0.0f,           0.0f,
         1.0f, 1.0f, -4.0f, (4.0f / 32.0f),  0.0f,
         1.0f, 1.0f,  4.0f, (4.0f / 32.0f), (12.0f / 16.0f),
         1.0f, 1.0f,  4.0f, (4.0f / 32.0f), (12.0f / 16.0f),
        -1.0f, 1.0f,  4.0f,  0.0f,          (12.0f / 16.0f),
        -1.0f, 1.0f, -4.0f,  0.0f,           0.0f,

        // Back
        -1.0f, -1.0f, -4.0f,  0.0f,           0.0f,
         1.0f, -1.0f, -4.0f, (4.0f / 32.0f),  0.0f,
         1.0f,  1.0f, -4.0f, (4.0f / 32.0f), (4.0f / 16.0f),
         1.0f,  1.0f, -4.0f, (4.0f / 32.0f), (4.0f / 16.0f),
        -1.0f,  1.0f, -4.0f,  0.0f,          (4.0f / 16.0f),
        -1.0f, -1.0f, -4.0f,  0.0f,           0.0f,
    };
    pistol_base_mesh_.SetShader(ShaderManager::MOB_SHADER);
    pistol_base_mesh_.SetVertexData(pistol_base_vertices, sizeof(pistol_base_vertices) / (5 * sizeof(float)));
    pistol_base_mesh_.SetTexture(Storage::IMAGES / "entities" / "player_pistol.png");

    // Pistol slide mesh
    float pistol_slide_vertices[] = {
        // Back
        -1.0f, -1.0f, -1.0f, 0.5f, 0.0f,
         1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 0.5f, 0.0f,

        // Side
        -1.0f, -1.0f,  1.0f, 0.5f, 0.0f,
        -1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f, 0.5f, 0.0f,

        // Top
        -1.0f, 1.0f, -1.0f, 0.5f, 0.0f,
         1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
         1.0f, 1.0f,  1.0f, 1.0f, 1.0f,
         1.0f, 1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 0.5f, 0.0f,
    };
    pistol_slide_mesh_.SetShader(ShaderManager::MOB_SHADER);
    pistol_slide_mesh_.SetVertexData(pistol_slide_vertices, sizeof(pistol_slide_vertices) / (5 * sizeof(float)));
    pistol_slide_mesh_.SetTexture(Storage::IMAGES / "entities" / "player_pistol.png");

    // Sprite mesh
    float sprite_vertices[] = {
        -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         1.0f, 2.0f, 0.0f, 1.0f, 1.0f,
         1.0f, 2.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 2.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    };
    sprite_mesh_.SetShader(ShaderManager::MOB_SHADER);
    sprite_mesh_.SetVertexData(sprite_vertices, sizeof(sprite_vertices) / (5 * sizeof(float)));

    // Block mesh
    block_mesh_.SetShader(ShaderManager::MOB_SHADER);
    block_mesh_.SetTexture(Storage::IMAGES / "texture_atlas.png");
}

void Player::Update(float delta_time)
{
    input_direction_ = glm::vec3(0);

    auto selected_item = inventory_.GetSelectedItem();

    if (pain_time_ != 0)
    {
        pain_time_ -= delta_time;
        if (pain_time_ <= 0)
            pain_time_ = 0;
    }

    if (in_control_)
    {
        auto forward = GetForward();
        auto right = GetRight();
        if (Input::IsKeyHeld(GLFW_KEY_W))
            input_direction_ += forward;
        if (Input::IsKeyHeld(GLFW_KEY_S))
            input_direction_ -= forward;
        if (Input::IsKeyHeld(GLFW_KEY_A))
            input_direction_ -= right;
        if (Input::IsKeyHeld(GLFW_KEY_D))
            input_direction_ += right;
        if (Input::IsKeyPressed(GLFW_KEY_SPACE) && is_grounded_)
            is_jumping_ = true;

        if (glm::length(input_direction_) > 0)
            input_direction_ = glm::normalize(input_direction_);

        // Use medkit
        if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) && selected_item == ItemID::medkit && health_ < 100)
        {
            health_ += 25 + RNG{}.Range(0, 9);
            health_ = glm::clamp(health_, 0, 100);
            inventory_.inventory[0][inventory_.selected_hotbar_slot] = {ItemID::none, 0};
            SoundSystem::Play(SoundSystem::Sound::MEDKIT);
        }

        // Decide whether we're flying
        if (Input::IsKeyHeld(GLFW_KEY_SPACE) && jetpack_energy_ > 0)
        {
            time_since_started_flying_ += delta_time;
            if (time_since_started_flying_ > 0.5f)
            {
                is_flying_ = true;

                if (jetpack_sound_ == nullptr)
                    jetpack_sound_ = SoundSystem::PlayLooped(SoundSystem::Sound::JETPACK);
            }
        }
        else if (Input::IsKeyReleased(GLFW_KEY_SPACE) || jetpack_energy_ < 1)
        {
            time_since_started_flying_ = 0;
            is_flying_ = false;

            if (jetpack_sound_ != nullptr)
            {
                jetpack_sound_->source->Stop();
                jetpack_sound_ = nullptr;
            }
        }

        // Punching
        bool punch_mining = Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT) && !ItemIsDrill(selected_item) && !ItemIsPistol(selected_item);
        bool placing_block = Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT) && ItemIsBlock(selected_item);
        if (punch_mining || placing_block)
        {
            time_punching_ += delta_time;
            arm_extent_ = 0.2f * glm::pow(glm::sin(7.0f * time_punching_), 2);
        }
        else if (time_punching_ != 0) // Animation should stop
        {
            if (glm::abs(arm_extent_) < 0.01f)
            {
                arm_extent_ = 0;
                time_punching_ = 0;
            }
            else
            {
                time_punching_ += delta_time;
                arm_extent_ = 0.2f * glm::pow(glm::sin(7.0f * time_punching_), 2);
            }
        }

        // Drilling
        if (ItemIsDrill(selected_item))
        {
            if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                if (drill_sound_ == nullptr)
                {
                    if (selected_item == ItemID::drill_t3)
                        drill_sound_ = SoundSystem::PlayLooped(SoundSystem::Sound::DRILL3);
                    else
                        drill_sound_ = SoundSystem::PlayLooped(SoundSystem::Sound::DRILL);
                }
            }
            else if (Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT))
            {
                time_drilling_ += delta_time;

                float arm_shake_freq = glm::clamp(15.0f * time_drilling_, 20.0f, 50.0f);
                arm_shake_ = 0.005f * glm::pow(glm::sin(arm_shake_freq * time_drilling_), 2);
                arm_extent_ = glm::clamp(arm_extent_ + 0.0001f * time_drilling_, 0.0f, 0.2f);

                drill_bit_angular_speed_ = glm::clamp(3.0f * time_drilling_, 0.0f, 40.0f);
                drill_bit_rotation_ = drill_bit_angular_speed_ * time_drilling_;
                drill_bit_extent_ = glm::clamp(drill_bit_extent_ + 0.0001f * time_drilling_, 0.0f, 0.2f);
            }
            else if (Input::IsMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT) || time_drilling_ != 0)
            {
                if (drill_sound_ != nullptr)
                {
                    drill_sound_->source->Stop();
                    drill_sound_ = nullptr;
                }

                arm_shake_ = 0;

                if (glm::abs(arm_extent_) < 0.01f)
                {
                    arm_extent_ = 0;
                    arm_shake_ = 0;
                    drill_bit_extent_ = 0;
                    drill_bit_rotation_ = 0;
                }
                else
                {
                    // For the sake of simplicity, we now reinterpret time_drilling_ as the time
                    // elapsed since the animation started ending
                    if (time_drilling_ != 0)
                        time_drilling_ = 0;
                        
                    time_drilling_ += delta_time;

                    arm_extent_ = glm::clamp(arm_extent_ - 0.2f * time_drilling_, 0.0f, 2.0f);

                    drill_bit_rotation_ += drill_bit_angular_speed_ * time_drilling_;
                    drill_bit_extent_ = glm::clamp(drill_bit_extent_ - 0.2f * time_drilling_, 0.0f, 0.2f);
                }
            }
        }

        // Pistol
        if (ItemIsPistol(selected_item))
        {
            if (Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT))
            {
                float scale = 1.0f;
                if (selected_item == ItemID::slug_pistol_t2)
                    scale = 2.0f;
                else if (selected_item == ItemID::slug_pistol_t3)
                    scale = 4.0f;

                time_charging_gun_ += scale * delta_time;

                pistol_base_disp_ = 0.01f * glm::sin(50.0f * (time_charging_gun_ / scale));
                pistol_slide_disp_ = 0.4f * (1 - glm::exp(-0.5f * time_charging_gun_));
            }
            else if (time_charging_gun_ != 0)
            {
                glm::vec3 offset = 2.0f * camera_.forward + 0.5f * camera_.right - 0.1f * camera_.up;

                float speed = 50.0f * glm::clamp(time_charging_gun_ * time_charging_gun_, 0.0f, 16.0f);
                if (selected_item == ItemID::slug_pistol_t2)
                    speed *= 2.0f;
                else if (selected_item == ItemID::slug_pistol_t3)
                    speed *= 3.0f;

                int damage_param = selected_item == ItemID::slug_pistol_t3 ? 4  // This is how slug damage was calculated in the latest
                                 : selected_item == ItemID::slug_pistol_t2 ? 2  // version (v2.01) of the original game
                                 :                                           0;

                Slug *slug = new Slug({
                    .initial_position = camera_.position + offset,
                    .initial_velocity = speed * camera_.forward,
                    .source_id = GetID(),
                    .damage = damage_param * 8 + 20
                });
                Moon::GetCurrentMoon()->GetEntityManager().AddEntity(slug);

                SoundSystem::Play(SoundSystem::Sound::LASER);

                time_charging_gun_ = 0;
                pistol_base_disp_ = 0;
                pistol_slide_disp_ = 0;
            }
        }
    }

    // Regen health
    if (suit_status_ > 0 && time_since_last_health_update_ > 0.5f)
    {
        health_ = glm::clamp(health_ + 1, 0, 100);
        time_since_last_health_update_ = 0;
    }
    else
    {
        time_since_last_health_update_ += delta_time;
    }

    // Regen suit status
    if (CanRegenSuit() && time_since_last_suit_update_ > GetSuitRegenInterval())
    {
        suit_status_ = glm::clamp(suit_status_ + 1, 0, 100);
        time_since_last_suit_update_ = 0;
    }
    else
    {
        time_since_last_suit_update_ += delta_time;
    }

    // Deplete/regen jetpack energy
    if (is_flying_ && time_since_last_jetpack_update_ > 0.055f)
    {
        jetpack_energy_ = glm::clamp(jetpack_energy_ - 1, 0, GetMaxJetpackEnergy());
        time_since_last_jetpack_update_ = 0;
    }
    else if (!is_flying_ && time_since_last_jetpack_update_ > 1.0f)
    {
        jetpack_energy_ = glm::clamp(jetpack_energy_ + 1, 0, GetMaxJetpackEnergy());
        time_since_last_jetpack_update_ = 0;
    }
    else
    {
        time_since_last_jetpack_update_ += delta_time;
    }

    // Walking (bob animations)
    if (glm::length(input_direction_) > 0 && is_grounded_)
    {
        time_walking_ += delta_time;
        arm_bob_ = 0.01f * glm::pow(glm::sin(5.0f * time_walking_), 2);
        camera_bob_ = 0.04f * glm::sin(10.0f * time_walking_ + 3.1415f);
    }
    else if (time_walking_ != 0)
    {
        if (glm::abs(arm_bob_) < 0.001f && glm::abs(camera_bob_) < 0.01f)
        {
            time_walking_ = 0;
            arm_bob_ = 0;
            camera_bob_ = 0;
        }
        else
        {
            time_walking_ += delta_time;
            arm_bob_ = 0.01f * glm::pow(glm::sin(5.0f * time_walking_), 2);
            camera_bob_ = 0.04f * glm::sin(10.0f * time_walking_ + 3.1415f);
        }
    }

    // Camera shake when flying
    if (is_flying_)
        time_flying_ += delta_time;
    else
        time_flying_ = 0;
    camera_.pitch += 0.005f * glm::sin(80.0f * time_flying_);

    camera_.position = position_ + glm::vec3(0, 0.9f + camera_bob_, 0);
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

    velocity_.x += (alpha * input_direction_.x - beta * velocity_.x) * FIXED_DELTA_TIME;
    if (glm::abs(velocity_.x) < 0.01f)
        velocity_.x = 0;

    velocity_.z += (alpha * input_direction_.z - beta * velocity_.z) * FIXED_DELTA_TIME;
    if (glm::abs(velocity_.z) < 0.01f)
        velocity_.z = 0;

    //
    // Jumping/flying
    //

    if (is_jumping_ && is_grounded_)
    {
        SoundSystem::Play(SoundSystem::Sound::JUMP);
        velocity_.y = 3.5f;
        is_jumping_ = false;
    }

    if (is_flying_)
    {
        velocity_.y = (4.0f * FIXED_DELTA_TIME) + 6.0f; // First term counteracts gravity
    }

    // Play land sound when landing
    if (!was_grounded_ && is_grounded_)
    {
        if (fall_time_ >= 2.0f)
            SoundSystem::Play(SoundSystem::Sound::LAND);
        fall_time_ = 0;
    }
    else if (!was_grounded_)
    {
        fall_time_ += FIXED_DELTA_TIME;
    }
    was_grounded_ = is_grounded_;
}

bool Player::IsInControl()
{
    return in_control_;
}

void Player::EnableControl()
{
    in_control_ = true;
}

void Player::DisableControl()
{
    in_control_ = false;
}

void Player::UpdateCamera()
{
    if (in_control_)
    {
        auto mouse_delta = Input::GetMouseDelta();
        camera_.yaw += mouse_delta.x * camera_.sensitivity;
        camera_.pitch += mouse_delta.y * camera_.sensitivity;
        camera_.pitch = glm::clamp(camera_.pitch, -89.8f, 89.8f);

        glm::vec3 direction;
        direction.x = cos(glm::radians(camera_.yaw)) * cos(glm::radians(camera_.pitch));
        direction.y = sin(glm::radians(camera_.pitch));
        direction.z = sin(glm::radians(camera_.yaw)) * cos(glm::radians(camera_.pitch));
        camera_.forward = glm::normalize(direction);
        camera_.right = glm::normalize(glm::cross(camera_.forward, camera_.up));
    }
}

PlayerData Player::GetPlayerData()
{
    PlayerData player_data;
    player_data.health = health_;
    player_data.suit_status = suit_status_;
    player_data.jetpack_energy = jetpack_energy_;
    player_data.position = position_;
    player_data.camera_rotation = GetCameraRotation();
    player_data.inventory = inventory_;
    return player_data;
}

Camera &Player::GetCamera()
{
    return camera_;
}

void Player::SetSuitStatus(int suit_status)
{
    suit_status_ = suit_status;
}

int Player::GetSuitStatus()
{
    return suit_status_;
}

Inventory &Player::GetInventory()
{
    return inventory_;
}

void Player::SetInventory(Inventory inventory)
{
    inventory_ = inventory;
}

void Player::SetCameraRotation(glm::vec2 rotation)
{
    camera_.pitch = rotation.x;
    camera_.yaw = rotation.y;
}

glm::vec2 Player::GetCameraRotation()
{
    return {camera_.pitch, camera_.yaw};
}

void Player::SetCameraSensitivity(float sensitivity)
{
    camera_.SetSensitivity(sensitivity);
}

glm::vec3 Player::GetForward()
{
    return {
        glm::cos(glm::radians(camera_.yaw)),
        0,
        glm::sin(glm::radians(camera_.yaw))
    };
}

glm::vec3 Player::GetRight()
{
    return glm::cross(GetForward(), glm::vec3{0, 1, 0});
}

void Player::SetHealth(int health) noexcept
{
    if (health < health_)
    {
        pain_time_ = 0.5f;

        int damage = health_ - health;
        int divided_damage = damage / 2;

        if (suit_status_ < divided_damage)
        {
            int leftover = divided_damage - suit_status_;
            suit_status_ = 0;
            health_ -= divided_damage + leftover;
        }
        else
        {
            suit_status_ -= divided_damage;
            health_ -= damage - divided_damage;
        }

        if (health_ <= 0)
        {
            health_ = 0;
            SetIsDead(true);
        }
    }
    else
    {
        health_ = health;
    }
}

void Player::Render(const glm::mat4 &vp_matrix)
{
    auto inv_view = glm::inverse(camera_.GetViewMatrix()); // To do this in camera space

    Shader &shader = ShaderManager::MOB_SHADER;
    shader.Use();
    shader.SetMat4("u_vp_matrix", vp_matrix * inv_view);
    shader.SetVec3("u_ws_camera_position", camera_.position);
    shader.SetVec4("u_color", {1.0f, 0.0f, 0.0f, pain_time_});
    shader.SetVec4("u_fog_color", glm::vec4{0.0f});
    shader.SetFloat("u_fog_distance", 0);

    // Arm
    auto arm_model_matrix = glm::mat4(1.0);
    arm_model_matrix = glm::translate(arm_model_matrix, {0.48f + arm_shake_, -0.35f + arm_bob_, -0.35f - arm_extent_});
    arm_model_matrix = glm::scale(arm_model_matrix, {-0.45f, 0.45f, -0.45f});
    arm_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model_matrix", arm_model_matrix);
    });

    ItemID selected_item = inventory_.GetSelectedItem();
    if (selected_item != ItemID::none)
    {
        if (selected_item == ItemID::slug_pistol_t1 || selected_item == ItemID::slug_pistol_t2 || selected_item == ItemID::slug_pistol_t3)
        {
            // Pistol base
            auto pistol_base_model_matrix = glm::mat4(1.0);
            pistol_base_model_matrix = glm::translate(pistol_base_model_matrix, {0.46f, -0.15f + arm_bob_, -1.2f + pistol_base_disp_});
            pistol_base_model_matrix = glm::scale(pistol_base_model_matrix, {0.08f, 0.08f, -0.08f});
            pistol_base_mesh_.Render([&](Shader *shader) {
                shader->SetMat4("u_model_matrix", pistol_base_model_matrix);
            });

            // Pistol slide
            auto pistol_slide_model_matrix = glm::mat4(1.0);
            pistol_slide_model_matrix = glm::translate(pistol_slide_model_matrix, {0.44f, -0.06f + arm_bob_, -1.35f + pistol_base_disp_ + pistol_slide_disp_});
            pistol_slide_model_matrix = glm::scale(pistol_slide_model_matrix, {0.03f, 0.02f, -0.02f});
            pistol_slide_mesh_.Render([&](Shader *shader) {
                shader->SetMat4("u_model_matrix", pistol_slide_model_matrix);
            });
        }
        else if (selected_item == ItemID::drill_t1 || selected_item == ItemID::drill_t2 || selected_item == ItemID::drill_t3)
        {
            // Drill base
            auto drill_base_model_matrix = glm::mat4(1.0);
            drill_base_model_matrix = glm::translate(drill_base_model_matrix, {0.5f + arm_shake_, -0.35f + arm_bob_, -1.0f - arm_extent_});
            drill_base_model_matrix = glm::scale(drill_base_model_matrix, {0.15f, 0.15f, -0.15f});
            drill_base_mesh_.Render([&](Shader *shader) {
                shader->SetMat4("u_model_matrix", drill_base_model_matrix);
            });

            // Drill bit
            auto drill_bit_model_matrix = glm::mat4(1.0);
            drill_bit_model_matrix = glm::translate(drill_bit_model_matrix, {0.5f + arm_shake_, -0.35f + arm_bob_, -1.5f - arm_extent_ - drill_bit_extent_});
            drill_bit_model_matrix = glm::rotate(drill_bit_model_matrix, drill_bit_rotation_, {0, 0, 1});
            drill_bit_model_matrix = glm::scale(drill_bit_model_matrix, {0.05f, 0.05f, -0.05f});
            drill_bit_mesh_.Render([&](Shader *shader) {
                shader->SetMat4("u_model_matrix", drill_bit_model_matrix);
            });
        }
        else if (ItemIsSprite(selected_item) || selected_item == ItemID::minilight)
        {
            auto sprite_model_matrix = glm::mat4(1.0);
            sprite_model_matrix = glm::translate(sprite_model_matrix, {0.362f, -0.335f + arm_bob_, -0.95f - arm_extent_});
            sprite_model_matrix = glm::scale(sprite_model_matrix, {0.14f, 0.14f, -0.14f});

            if (last_held_sprite_ != selected_item)
            {
                sprite_mesh_.SetTexture(Storage::IMAGES / "items" / GetItemFile(selected_item));
                last_held_sprite_ = selected_item;
            }
            sprite_mesh_.Render([&](Shader *shader) {
                shader->SetMat4("u_model_matrix", sprite_model_matrix);
            });
        }
        else
        {
            auto block_model_matrix = glm::mat4(1.0);
            block_model_matrix = glm::translate(block_model_matrix, {0.38f, -0.25f + arm_bob_, -1.0f - arm_extent_});
            block_model_matrix = glm::scale(block_model_matrix, {0.14f, 0.14f, -0.14f});
            
            if (last_held_block_ != selected_item)
            {
                auto tile_origins = GetAtlasTileOrigins();
                glm::vec2 tile_origin_side = tile_origins[ItemIDToBlockID(selected_item)][1];
                glm::vec2 tile_origin_top = tile_origins[ItemIDToBlockID(selected_item)][0];
                float tile_size = 1.0f / 14.0f;
                float block_vertices[] = {
                    // Back
                    -1.0f, -1.0f, -1.0f, tile_origin_side.x,             tile_origin_side.y,
                     1.0f, -1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
                     1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,
                     1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,
                    -1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
                    -1.0f, -1.0f, -1.0f, tile_origin_side.x,             tile_origin_side.y,

                    // Side
                    -1.0f, -1.0f,  1.0f, tile_origin_side.x,             tile_origin_side.y,
                    -1.0f, -1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
                    -1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,
                    -1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,
                    -1.0f,  1.0f,  1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
                    -1.0f, -1.0f,  1.0f, tile_origin_side.x,             tile_origin_side.y,

                    // Top
                    -1.0f, 1.0f, -1.0f, tile_origin_top.x,             tile_origin_top.y,
                     1.0f, 1.0f, -1.0f, tile_origin_top.x + tile_size, tile_origin_top.y,
                     1.0f, 1.0f,  1.0f, tile_origin_top.x + tile_size, tile_origin_top.y + tile_size,
                     1.0f, 1.0f,  1.0f, tile_origin_top.x + tile_size, tile_origin_top.y + tile_size,
                    -1.0f, 1.0f,  1.0f, tile_origin_top.x + tile_size, tile_origin_top.y,
                    -1.0f, 1.0f, -1.0f, tile_origin_top.x,             tile_origin_top.y,
                };
                block_mesh_.SetVertexData(block_vertices, sizeof(block_vertices) / (5 * sizeof(float)));
                last_held_block_ = selected_item;
            }
            block_mesh_.Render([&](Shader *shader) {
                shader->SetMat4("u_model_matrix", block_model_matrix);
            });
        }
    }
}

// In blocks/second
float Player::GetMaxMoveSpeed()
{
    float max_speed;

    auto battery_item = inventory_.spacesuit[1].item;
    const float base_speed = 3.5f;
    if (battery_item == ItemID::battery)
        max_speed = 1.333f * base_speed;                      // These are the actual ratios from version 2.01 of the original game
    else if (battery_item == ItemID::power_crystal)           //
        max_speed = 1.375f * 1.333f * base_speed;
    else if (battery_item == ItemID::energy_orb)
        max_speed = 1.455f * 1.375f * 1.333f * base_speed;
    else
        max_speed = base_speed;

    if (IsOnIce())
        max_speed *= 1.5f;

    return max_speed;
}

bool Player::CanRegenSuit()
{
    auto battery_item = inventory_.spacesuit[1].item;
    return battery_item == ItemID::battery || battery_item == ItemID::power_crystal || battery_item == ItemID::energy_orb;
}

// In seconds
float Player::GetSuitRegenInterval()
{
    auto battery_item = inventory_.spacesuit[1].item;
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
    jetpack_energy_ = energy;
}

int Player::GetJetpackEnergy()
{
    return jetpack_energy_;
}

int Player::GetMaxJetpackEnergy()
{
    auto jetpack_item = inventory_.spacesuit[0].item;
    if (jetpack_item == ItemID::jetpack_t1)
        return 20;
    else if (jetpack_item == ItemID::jetpack_t2)
        return 40;
    else if (jetpack_item == ItemID::jetpack_t3)
        return 75;
    else
        return 0;
}
