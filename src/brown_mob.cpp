
#include "brown_mob.h"
#include "moon.h"
#include "options.h"
#include "storage.h"
#include "rng.h"
#include "dropped_item.h"

BrownMob::BrownMob(BrownMobData data)
{
    type_ = EntityType::BROWN_MOB;
    position_ = data.position;
    prev_position_ = data.position;
    next_position_ = data.position;
    yaw_ = data.yaw;
    health_ = data.health;
    aabb_.center = data.position;
    aabb_.extents = {0.5f, 0.25f, 0.5f};
    death_animation_done_ = true;
    aggressive_ = false;

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
    if (IsDead())
        time_since_death_ += delta_time;

    if (pain_time_ != 0)
    {
        pain_time_ -= delta_time;
        if (pain_time_ < 0)
            pain_time_ = 0;
    }

    if (is_grounded_ && !aggressive_)
    {
        if (next_action_time_ <= 0)
        {
            next_action_time_ = RNG{}.Range(1.0f, 3.0f);
            if (RNG{}.Range(0, 1) == 0)
            {
                action_ = BrownMobAction::JUMP;

                if (RNG{}.Range(0, 2) == 0) // Hop
                {
                    jump_vector_ = RNG{}.Range(1.0f, 4.0f) * glm::vec3{0, 1, 0};
                }
                else // Jump forward
                {
                    auto yaw_rotation = glm::mat3{glm::rotate(glm::mat4{1.0f}, glm::radians(yaw_), {0, 1, 0})};
                    glm::vec3 forward = yaw_rotation * glm::vec3{0.0f, 0.0f, 1.0f};
                    jump_vector_ = RNG{}.Range(2.0f, 8.0f) * forward
                                 + RNG{}.Range(6.0f, 16.0f) * glm::vec3{0, 1, 0};
                }
            }
            else
            {
                action_ = BrownMobAction::ROTATE;

                target_yaw_ = RNG{}.Range(10.0f, 90.0f);
                if (RNG{}.Range(0, 1) == 0)
                    target_yaw_ *= -1;
            }
        }
        else if (action_ == BrownMobAction::NONE)
        {
            next_action_time_ -= delta_time;
        }
    }
}

void BrownMob::FixedUpdate()
{
    if (IsDead())
    {
        velocity_.x = 0;
        velocity_.z = 0;
    }
    else if (aggressive_)
    {
        Entity *target = Moon::GetCurrentMoon()->GetEntityManager().GetEntityByID(target_id_);
        if (target)
        {
            time_chasing_ += FIXED_DELTA_TIME;

            auto displacement = target->GetPosition() - position_;
            auto horizontal_displacement = glm::vec3{displacement.x, 0, displacement.z};
            float horizontal_distance = glm::length(horizontal_displacement);

            if (glm::length(displacement) < 0.8f)
            {
                target->SetHealth(target->GetHealth() - 25);
                Moon::GetCurrentMoon()->BrownMobExplode(position_);
                SetIsDead(true);
                return;
            }

            yaw_ = glm::degrees(glm::acos(glm::normalize(horizontal_displacement).z));
            if (horizontal_displacement.x < 0)
                yaw_ *= -1;

            if (time_chasing_ < 1.0f)
            {
                auto right = glm::normalize(glm::cross(horizontal_displacement, {0, 1, 0}));
                velocity_ -= (8.0f * glm::normalize(horizontal_displacement) + 6.0f * glm::sin(1.0f * time_chasing_) * right) * FIXED_DELTA_TIME;
            }
            else if (horizontal_distance > 1.0f)
            {
                velocity_ += 8.0f * glm::normalize(horizontal_displacement) * FIXED_DELTA_TIME;
                
                float y = velocity_.y;
                velocity_ = glm::dot(glm::vec3{velocity_.x, 0, velocity_.z}, glm::normalize(horizontal_displacement)) * glm::normalize(horizontal_displacement);
                velocity_.y = y;
            }

            velocity_.x = glm::clamp(velocity_.x, -8.0f, 8.0f);
            velocity_.z = glm::clamp(velocity_.z, -8.0f, 8.0f);
        }
        else
        {
            aggressive_ = false;
        }
    }
    else if (action_ == BrownMobAction::JUMP)
    {
        if (glm::length(jump_vector_) > 0)
        {
            velocity_ = jump_vector_;
            jump_vector_ = glm::vec3{0};
        }

        if (is_grounded_)
        {
            // Ad-hoc friction
            if (glm::abs(velocity_.y) < 0.01f)
                velocity_ *= IsOnIce() ? 0.99f : 0.95f;

            // Allow it to slide for a bit before stopping
            if (glm::length(velocity_) < 0.1f)
            {
                velocity_ = glm::vec3{0};
                action_ = BrownMobAction::NONE;
            }
        }
    }
    else if (action_ == BrownMobAction::ROTATE)
    {
        if (target_yaw_ > 0)
        {
            float delta = 103.0f * FIXED_DELTA_TIME;
            yaw_ += delta;
            target_yaw_ -= delta;
        }
        else
        {
            action_ = BrownMobAction::NONE;
        }
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
        shader->SetVec4("u_color", {1.0f, 0.0f, 0.0f, pain_time_});
    });
}

BrownMobData BrownMob::GetBrownMobData()
{
    return {
        .position = position_,
        .yaw = yaw_,
        .health = health_
    };
}

void BrownMob::NotifyOfAttacker(size_t id)
{
    if (!aggressive_)
    {
        target_id_ = id;
        aggressive_ = true;
    }
}
