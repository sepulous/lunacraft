
#include "brown_mob.h"
#include "moon.h"
#include "options.h"
#include "storage.h"
#include "rng.h"
#include "dropped_item.h"
#include "sound_system.h"

BrownMob::BrownMob(BrownMobData data)
{
    id_ = data.id;
    type_ = EntityType::BROWN_MOB;
    position_ = data.position;
    prev_position_ = data.position;
    next_position_ = data.position;
    yaw_ = data.yaw;
    health_ = data.health;
    target_entity_id_ = data.target_entity_id;
    action_ = static_cast<BrownMobAction>(data.action);
    aabb_.center = data.position;
    aabb_.extents = {0.5f, 0.25f, 0.5f};
    death_animation_done_ = true;

    const float face_bottom = 1.0f / 3.0f;
    const float face_side_height = 1.0f / 6.0f;
    const float side_bottom = 0.5f;
    const float top_bottom = 2.0f / 3.0f;
    const float bottom_height = 1.0f / 3.0f;
    float vertices[] = {
        // Front
        -1.0f, -1.0f, 1.0f, 0.0f, face_bottom,
         1.0f, -1.0f, 1.0f, 1.0f, face_bottom,
         1.0f,  1.0f, 1.0f, 1.0f, face_bottom + face_side_height,
         1.0f,  1.0f, 1.0f, 1.0f, face_bottom + face_side_height,
        -1.0f,  1.0f, 1.0f, 0.0f, face_bottom + face_side_height,
        -1.0f, -1.0f, 1.0f, 0.0f, face_bottom,

        // Back
         1.0f,  1.0f, -1.0f, 1.0f, side_bottom + face_side_height,
         1.0f, -1.0f, -1.0f, 1.0f, side_bottom,
        -1.0f, -1.0f, -1.0f, 0.0f, side_bottom,
        -1.0f, -1.0f, -1.0f, 0.0f, side_bottom,
        -1.0f,  1.0f, -1.0f, 0.0f, side_bottom + face_side_height,
         1.0f,  1.0f, -1.0f, 1.0f, side_bottom + face_side_height,

        // Left
        -1.0f,  1.0f, -1.0f, 1.0f, side_bottom + face_side_height,
        -1.0f, -1.0f, -1.0f, 1.0f, side_bottom,
        -1.0f, -1.0f,  1.0f, 0.0f, side_bottom,
        -1.0f, -1.0f,  1.0f, 0.0f, side_bottom,
        -1.0f,  1.0f,  1.0f, 0.0f, side_bottom + face_side_height,
        -1.0f,  1.0f, -1.0f, 1.0f, side_bottom + face_side_height,

        // Right
        1.0f, -1.0f,  1.0f, 0.0f, side_bottom,
        1.0f, -1.0f, -1.0f, 1.0f, side_bottom,
        1.0f,  1.0f, -1.0f, 1.0f, side_bottom + face_side_height,
        1.0f,  1.0f, -1.0f, 1.0f, side_bottom + face_side_height,
        1.0f,  1.0f,  1.0f, 0.0f, side_bottom + face_side_height,
        1.0f, -1.0f,  1.0f, 0.0f, side_bottom,

        // Top
         1.0f, 1.0f,  1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, -1.0f, 1.0f, top_bottom,
        -1.0f, 1.0f, -1.0f, 0.0f, top_bottom,
        -1.0f, 1.0f, -1.0f, 0.0f, top_bottom,
        -1.0f, 1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, 1.0f,  1.0f, 1.0f, 1.0f,

        // Bottom
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 1.0f, bottom_height,
         1.0f, -1.0f,  1.0f, 1.0f, bottom_height,
        -1.0f, -1.0f,  1.0f, 0.0f, bottom_height,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
    };
    mesh_.SetShader(ShaderManager::MOB_SHADER);
    mesh_.SetTexture(Storage::IMAGES / "entities" / "brown_mob.png");
    mesh_.SetVertexData(vertices, sizeof(vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);
}

void BrownMob::Update(float delta_time)
{
    internal_time_ += delta_time;

    if (IsDead())
        time_since_death_ += delta_time;

    if (pain_time_ != 0)
    {
        pain_time_ -= delta_time;
        if (pain_time_ < 0)
            pain_time_ = 0;
    }

    yaw_ = glm::mod(yaw_, 360.0f);
    if (yaw_ < 0.0f)
        yaw_ += 360.0f;

    if (internal_time_ > next_action_time_ && !IsDead())
    {
        move_velocity_ = glm::vec3{0};
        if (action_ == BrownMobAction::NONE)
        {
            if (!is_grounded_)
            {
                next_action_time_ += 1.0f;
            }
            else
            {
                float chance = RNG{}.Range(0.0f, 1.0f);
                if (chance < 0.3f)
                {
                    action_ = BrownMobAction::MOVE;
                    next_action_time_ += RNG{}.Range(1.0f, 4.0f);

                    velocity_.y += RNG{}.Range(4.0f, 8.0f);
                    move_velocity_ = 6.0f * glm::vec3{glm::sin(glm::radians(yaw_)), 0, glm::cos(glm::radians(yaw_))};

                    SoundSystem::PlayAt(SoundSystem::Sound::ALIEN_JUMP, position_);
                }
                else if (chance < 0.5f)
                {
                    action_ = BrownMobAction::ROTATE_LEFT;
                    next_action_time_ += RNG{}.Range(0.0f, 0.5f);
                }
                else if (chance < 0.7f)
                {
                    action_ = BrownMobAction::ROTATE_RIGHT;
                    next_action_time_ += RNG{}.Range(0.0f, 0.5f);
                }
                else if (chance < 0.9f)
                {
                    auto moon = Moon::GetCurrentMoon();
                    auto player = moon->GetPlayer();
                    auto player_distance = glm::length(player->GetPosition() - position_);
                    if (!moon->GetSettings().is_creative && player_distance < 15.0f)
                    {
                        target_entity_id_ = player->GetID();
                        action_ = BrownMobAction::CHASE;
                        next_action_time_ += RNG{}.Range(0.0f, 0.5f);
                    }
                    else
                    {
                        action_ = BrownMobAction::MOVE;
                        next_action_time_ += RNG{}.Range(1.0f, 4.0f);

                        velocity_.y += RNG{}.Range(4.0f, 8.0f);
                        move_velocity_ = 0.6f * glm::vec3{glm::sin(glm::radians(yaw_)), 0, glm::cos(glm::radians(yaw_))};

                        SoundSystem::PlayAt(SoundSystem::Sound::ALIEN_JUMP, position_);
                    }
                }
            }
        }
        else if (action_ == BrownMobAction::CHASE)
        {
            Entity *target = Moon::GetCurrentMoon()->GetEntityManager().GetEntityByID(target_entity_id_);
            if (!target || target->IsDead())
            {
                action_ = BrownMobAction::NONE;
                next_action_time_ += 1.0f;
                return;
            }

            float distance = glm::length(target->GetPosition() - position_);

            if (distance > 50.0f)
            {
                action_ = BrownMobAction::NONE;
                next_action_time_ += 1.0f;
                return;
            }

            if (distance < 5.0)
            {
                SoundSystem::PlayAt(SoundSystem::Sound::BLOCK_BREAK, position_);
                target->Damage(25);
                Moon::GetCurrentMoon()->BrownMobExplode(position_);
                action_ = BrownMobAction::NONE;
                SetIsDead(true);
                return;
            }

            move_velocity_ = 10.0f * glm::vec3{glm::sin(glm::radians(yaw_)), 0, glm::cos(glm::radians(yaw_))};

            next_action_time_ += RNG{}.Range(1.0f, 1.5f);
        }
        else
        {
            action_ = BrownMobAction::NONE;
            next_action_time_ += RNG{}.Range(2.0f, 6.0f);
        }
    }

    if (!IsDead())
    {
        if (action_ == BrownMobAction::ROTATE_LEFT)
        {
            yaw_ -= 150.0f * delta_time;
        }
        else if (action_ == BrownMobAction::ROTATE_RIGHT)
        {
            yaw_ += 150.0f * delta_time;
        }
        else if (action_ == BrownMobAction::MOVE)
        {
            velocity_.x = move_velocity_.x;
            velocity_.z = move_velocity_.z;
        }
        else if (action_ == BrownMobAction::CHASE)
        {
            velocity_.x = move_velocity_.x;
            velocity_.z = move_velocity_.z;

            Entity *target = Moon::GetCurrentMoon()->GetEntityManager().GetEntityByID(target_entity_id_);
            if (target)
            {
                auto target_displacement = target->GetPosition() - position_;
                float target_yaw = glm::atan(target_displacement.z, target_displacement.x);
                float angle_deg = -glm::degrees(target_yaw) - 90.0f;
                float delta_angle = std::fmod((angle_deg + 720.0f - yaw_), 360.0f) - 180.0f;

                if (delta_angle < 0.0f)
                    yaw_ += std::max(-200.0f * delta_time, delta_angle);
                else
                    yaw_ += std::min(200.0f * delta_time, delta_angle);
            }
            else
            {
                action_ = BrownMobAction::NONE;
                next_action_time_ += 1.0f;
                return;
            }
        }
    }
}

void BrownMob::FixedUpdate()
{
    // Friction
    if (action_ == BrownMobAction::NONE && is_grounded_)
    {
        velocity_.x *= IsOnIce() ? 0.99f : 0.95f;
        if (glm::abs(velocity_.x) < 0.05f)
            velocity_.x = 0;

        velocity_.z *= IsOnIce() ? 0.99f : 0.95f;
        if (glm::abs(velocity_.z) < 0.05f)
            velocity_.z = 0;
    }
}

void BrownMob::Render(const glm::mat4 &view, const glm::mat4 &proj)
{
    glm::vec4 fog_color = Moon::GetCurrentMoon()->GetFogColor();
    float render_distance = OptionsManager::GetOptions().render_distance;

    glm::mat4 model{1.0f};
    model = glm::translate(model, position_);
    model = glm::rotate(model, glm::radians(yaw_), {0, 1, 0});
    model = glm::scale(model, glm::vec3{0.5f, 0.25f, 0.5f});

    mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", model);
        shader->SetMat4("u_view", view);
        shader->SetMat4("u_proj", proj);
        shader->SetVec4("u_fog_color", fog_color);
        shader->SetFloat("u_fog_start", (float)render_distance * 0.33f * 32.0f);
        shader->SetFloat("u_fog_end", (float)render_distance * 0.85f * 32.0f);
        shader->SetVec4("u_color", {1.0f, 1.0f - pain_time_, 1.0f - pain_time_, 1.0f});
    });
}

BrownMobData BrownMob::GetBrownMobData()
{
    return {
        .id = id_,
        .position = position_,
        .yaw = yaw_,
        .health = health_,
        .target_entity_id = target_entity_id_,
        .action = static_cast<uint8_t>(action_)
    };
}

void BrownMob::NotifyOfAttacker(size_t id)
{
    if (!IsDead() && action_ != BrownMobAction::CHASE)
    {
        target_entity_id_ = id;
        action_ = BrownMobAction::CHASE;
        next_action_time_ += 1.0f;

        Entity *target = Moon::GetCurrentMoon()->GetEntityManager().GetEntityByID(target_entity_id_);
        if (target)
        {
            auto recoil_dir = position_ - target->GetPosition();
            recoil_dir.y = 0;
            move_velocity_ = 5.0f * glm::normalize(recoil_dir);
        }
    }
}
