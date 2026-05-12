
#include "green_mob.h"
#include "moon.h"
#include "options.h"
#include "storage.h"
#include "rng.h"
#include "dropped_item.h"
#include "sound_system.h"

GreenMob::GreenMob(GreenMobData data)
{
    id_ = data.id;
    type_ = EntityType::GREEN_MOB;
    position_ = data.position;
    prev_position_ = data.position;
    next_position_ = data.position;
    yaw_ = data.yaw;
    roll_ = 0;
    health_ = data.health;
    aabb_.center = data.position;
    aabb_.extents = {0.5f, 0.5f, 0.5f};

    const float face_bottom = 2.0f / 3.0f;
    const float back_bottom = 1.0f / 3.0f;
    const float quad_height = 1.0f / 3.0f;
    float vertices[] = {
        // Front
        -1.0f, -1.0f, 1.0f, 0.0f, face_bottom,
         1.0f, -1.0f, 1.0f, 1.0f, face_bottom,
         1.0f,  1.0f, 1.0f, 1.0f, face_bottom + quad_height,
         1.0f,  1.0f, 1.0f, 1.0f, face_bottom + quad_height,
        -1.0f,  1.0f, 1.0f, 0.0f, face_bottom + quad_height,
        -1.0f, -1.0f, 1.0f, 0.0f, face_bottom,

        // Back
         1.0f,  1.0f, -1.0f, 1.0f, back_bottom + quad_height,
         1.0f, -1.0f, -1.0f, 1.0f, back_bottom,
        -1.0f, -1.0f, -1.0f, 0.0f, back_bottom,
        -1.0f, -1.0f, -1.0f, 0.0f, back_bottom,
        -1.0f,  1.0f, -1.0f, 0.0f, back_bottom + quad_height,
         1.0f,  1.0f, -1.0f, 1.0f, back_bottom + quad_height,

        // Left
        -1.0f,  1.0f, -1.0f, 1.0f, quad_height,
        -1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f, 0.0f, quad_height,
        -1.0f,  1.0f, -1.0f, 1.0f, quad_height,

        // Right
        1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
        1.0f,  1.0f, -1.0f, 1.0f, quad_height,
        1.0f,  1.0f, -1.0f, 1.0f, quad_height,
        1.0f,  1.0f,  1.0f, 0.0f, quad_height,
        1.0f, -1.0f,  1.0f, 0.0f, 0.0f,

        // Top
         1.0f, 1.0f,  1.0f, 1.0f, quad_height,
         1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f,  1.0f, 0.0f, quad_height,
         1.0f, 1.0f,  1.0f, 1.0f, quad_height,

        // Bottom
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 1.0f, quad_height,
         1.0f, -1.0f,  1.0f, 1.0f, quad_height,
        -1.0f, -1.0f,  1.0f, 0.0f, quad_height,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
    };
    mesh_.SetShader(ShaderManager::MOB_SHADER);
    mesh_.SetTexture(Storage::IMAGES / "entities" / "green_mob.png");
    mesh_.SetVertexData(vertices, sizeof(vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);
}

void GreenMob::Update(float delta_time)
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
        if (action_ == GreenMobAction::NONE)
        {
            if (!is_grounded_)
            {
                next_action_time_ += 1.0;
            }
            else
            {
                float chance = RNG{}.Range(0.0f, 1.0f);
                if (chance < 0.3)
                {
                    action_ = GreenMobAction::MOVE;
                    next_action_time_ += RNG{}.Range(1.0f, 4.0f);

                    velocity_.y += RNG{}.Range(4.0f, 10.0f);
                    move_velocity_ = 5.0f * glm::vec3{glm::sin(glm::radians(yaw_)), 0, glm::cos(glm::radians(yaw_))};

                    SoundSystem::PlayAt(SoundSystem::Sound::ALIEN_JUMP, position_);
                }
                else if (chance < 0.5)
                {
                    action_ = GreenMobAction::ROTATE_LEFT;
                    next_action_time_ += RNG{}.Range(0.0f, 0.5f);
                }
                else if (chance < 0.7)
                {
                    action_ = GreenMobAction::ROTATE_RIGHT;
                    next_action_time_ += RNG{}.Range(0.0f, 0.5f);
                }
                else if (chance < 0.9)
                {
                    action_ = GreenMobAction::MOVE;
                    next_action_time_ += RNG{}.Range(1.0f, 4.0f);

                    velocity_.y += RNG{}.Range(4.0f, 10.0f);
                    move_velocity_ = 0.5f * glm::vec3{glm::sin(glm::radians(yaw_)), 0, glm::cos(glm::radians(yaw_))};

                    SoundSystem::PlayAt(SoundSystem::Sound::ALIEN_JUMP, position_);
                }
            }
        }
        else
        {
            action_ = GreenMobAction::NONE;
            next_action_time_ += RNG{}.Range(2.0f, 6.0f);
        }
    }

    if (IsDead())
    {
        velocity_.x = 0;
        velocity_.z = 0;

        roll_ += 50.0 * delta_time;
        if (roll_ > 90.0)
            roll_ = 90.0;

        if (!dropped_biogel_)
        {
            DroppedItem *biogel = new DroppedItem({
                .position = position_,
                .item = ItemID::biogel,
                .amount = RNG{}.Range(1, 3)
            });
            biogel->SetVelocity({
                RNG{}.Range(-2.0f, 2.0f),
                RNG{}.Range(2.0f, 4.0f),
                RNG{}.Range(-2.0f, 2.0f)
            });
            Moon::GetCurrentMoon()->GetEntityManager().AddEntity(biogel);

            dropped_biogel_ = true;
        }

        if (time_since_death_ > 4.0f)
            death_animation_done_ = true;
    }
    else if (action_ == GreenMobAction::ROTATE_LEFT)
    {
        yaw_ -= 175.0f * delta_time;
    }
    else if (action_ == GreenMobAction::ROTATE_RIGHT)
    {
        yaw_ += 175.0f * delta_time;
    }
    else if (action_ == GreenMobAction::MOVE)
    {
        velocity_.x = move_velocity_.x;
        velocity_.z = move_velocity_.z;
    }
}

void GreenMob::FixedUpdate()
{
    // Friction
    if (action_ == GreenMobAction::NONE && is_grounded_)
    {
        velocity_.x *= IsOnIce() ? 0.99f : 0.95f;
        if (glm::abs(velocity_.x) < 0.05f)
            velocity_.x = 0;

        velocity_.z *= IsOnIce() ? 0.99f : 0.95f;
        if (glm::abs(velocity_.z) < 0.05f)
            velocity_.z = 0;
    }
}

void GreenMob::Render(const glm::mat4 &view, const glm::mat4 &proj)
{
    glm::vec4 fog_color = Moon::GetCurrentMoon()->GetFogColor();
    float render_distance = OptionsManager::GetOptions().render_distance;

    glm::mat4 model{1.0f};
    model = glm::translate(model, position_);
    model = glm::rotate(model, glm::radians(yaw_), {0, 1, 0});
    model = glm::rotate(model, glm::radians(roll_), {0, 0, 1});
    model = glm::scale(model, glm::vec3{0.5f});

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

GreenMobData GreenMob::GetGreenMobData()
{
    return {
        .id = id_,
        .position = position_,
        .yaw = yaw_,
        .health = health_
    };
}
