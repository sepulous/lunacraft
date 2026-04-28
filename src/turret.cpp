
#include "turret.h"
#include "moon.h"
#include "options.h"
#include "storage.h"
#include "rng.h"
#include "slug.h"
#include "sound_system.h"

Turret::Turret(TurretData data)
{
    id_ = data.id;
    type_ = EntityType::TURRET;
    position_ = data.position;
    prev_position_ = data.position;
    next_position_ = data.position;
    yaw_ = data.yaw;
    health_ = data.health;
    level_ = data.level;
    target_entity_id_ = data.target_entity_id;
    action_ = static_cast<TurretAction>(data.action);
    aabb_.center = data.position;
    aabb_.extents = {0.5f, 0.5f, 0.5f};
    death_animation_done_ = true;

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
    mesh_.SetTexture(Storage::IMAGES / "entities" / "turret.png");
    mesh_.SetVertexData(vertices, sizeof(vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);
}

void Turret::Update(float delta_time)
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
        if (action_ == TurretAction::NONE)
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
                    auto nearest_mob = Moon::GetCurrentMoon()->GetEntityManager().GetNearestMob(GetID(), (level_ + 1) * 15.0f);
                    if (nearest_mob)
                    {
                        target_entity_id_ = nearest_mob->GetID();
                        next_action_time_ += RNG{}.Range(0.0f, 0.5f);
                        action_ = TurretAction::ATTACK;
                    }
                    else
                    {
                        next_action_time_ += RNG{}.Range(1.0f, 4.0f);
                        action_ = TurretAction::NONE;
                    }
                }
                else if (chance < 0.5)
                {
                    action_ = TurretAction::ROTATE_LEFT;
                    next_action_time_ += RNG{}.Range(0.0f, 0.5f);
                }
                else if (chance < 0.7)
                {
                    action_ = TurretAction::ROTATE_RIGHT;
                    next_action_time_ += RNG{}.Range(0.0f, 0.5f);
                }
                else if (chance < 0.9)
                {
                    action_ = TurretAction::NONE;
                    next_action_time_ += RNG{}.Range(1.0f, 4.0f);
                }
            }
        }
        else if (action_ == TurretAction::ATTACK)
        {
            Entity *target = Moon::GetCurrentMoon()->GetEntityManager().GetEntityByID(target_entity_id_);
            if (!target || target->IsDead())
            {
                action_ = TurretAction::NONE;
                next_action_time_ += 1.0f;
                return;
            }

            float distance = glm::length(target->GetPosition() - position_);

            if (distance > 50.0f)
            {
                action_ = TurretAction::NONE;
                next_action_time_ += 1.0f;
                return;
            }

            // Aim
            auto displacement = target->GetPosition() - position_;
            float pitch = glm::asin(displacement.y / glm::length(displacement));

            // Build direction
            glm::vec3 dir {
                cos(pitch) * sin(glm::radians(yaw_)),
                sin(pitch),
                cos(pitch) * cos(glm::radians(yaw_))
            };

            // Speed scales with level
            float speed = RNG{}.Range(10.0f, 10.0f + level_ * 3.0f);

            // Final velocity (with gravity compensation)
            glm::vec3 velocity = speed * dir;
            velocity.y += 2.0f;

            // Fire
            Slug *slug = new Slug({
                .initial_position = position_,
                .initial_velocity = velocity,
                .source_id = GetID(),
                .damage = level_ * 8 + 20
            });
            Moon::GetCurrentMoon()->GetEntityManager().AddEntity(slug);
            SoundSystem::PlayAt(SoundSystem::Sound::LASER, position_);

            next_action_time_ += 3.0f / (level_ * 0.3f + 1.0f) + RNG{}.Range(0.0f, 0.8f);
        }
        else
        {
            action_ = TurretAction::NONE;
            next_action_time_ += RNG{}.Range(2.0f, 6.0f);
        }
    }

    if (IsDead())
    {
        velocity_.x = 0;
        velocity_.z = 0;
    }
    else if (action_ == TurretAction::ROTATE_LEFT)
    {
        yaw_ -= 100.0f * delta_time;
    }
    else if (action_ == TurretAction::ROTATE_RIGHT)
    {
        yaw_ += 100.0f * delta_time;
    }
    else if (action_ == TurretAction::ATTACK)
    {
        Entity *target = Moon::GetCurrentMoon()->GetEntityManager().GetEntityByID(target_entity_id_);
        if (target)
        {
            auto target_displacement = target->GetPosition() - position_;
            float target_yaw = glm::atan(target_displacement.z, target_displacement.x);
            float angle_deg = -glm::degrees(target_yaw) - 90.0f;
            float delta_angle = std::fmod((angle_deg + 720.0f - yaw_), 360.0f) - 180.0f;

            if (delta_angle < 0.0f)
                yaw_ += glm::max(-200.0f * delta_time, delta_angle);
            else
                yaw_ += glm::min(200.0f * delta_time, delta_angle);
        }
        else
        {
            action_ = TurretAction::NONE;
            next_action_time_ += 1.0f;
        }
    }
}

void Turret::FixedUpdate(){}

void Turret::Render(const glm::mat4 &view, const glm::mat4 &proj)
{
    glm::vec4 fog_color = Moon::GetCurrentMoon()->GetFogColor();
    float render_distance = OptionsManager::GetOptions().render_distance;

    glm::mat4 model{1.0f};
    model = glm::translate(model, position_);
    model = glm::rotate(model, glm::radians(yaw_), {0, 1, 0});
    model = glm::scale(model, glm::vec3{0.5f});

    glm::vec4 color;
    if (level_ == 0)
        color = {1.0f, 1.0f, 1.0f, 1.0f};
    else if (level_ == 2)
        color = {0.85f, 0.85f, 1.0f, 1.0f};
    else
        color = {1.0f, 1.0f, 0.6f, 1.0f};

    color.g *= 1.0f - pain_time_;
    color.b *= 1.0f - pain_time_;

    mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", model);
        shader->SetMat4("u_view", view);
        shader->SetMat4("u_proj", proj);
        shader->SetVec4("u_fog_color", fog_color);
        shader->SetFloat("u_fog_start", (float)render_distance * 0.33f * 32.0f);
        shader->SetFloat("u_fog_end", (float)render_distance * 0.85f * 32.0f);
        shader->SetVec4("u_color", color);
    });
}

TurretData Turret::GetTurretData()
{
    return {
        .id = id_,
        .position = position_,
        .yaw = yaw_,
        .health = health_,
        .level = level_,
        .target_entity_id = target_entity_id_,
        .action = static_cast<uint8_t>(action_)
    };
}

void Turret::NotifyOfAttacker(size_t id)
{
    if (action_ != TurretAction::ATTACK)
    {
        target_entity_id_ = id;
        action_ = TurretAction::ATTACK;
        next_action_time_ += RNG{}.Range(0.0f, 0.5f);
    }
}
