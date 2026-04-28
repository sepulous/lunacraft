
#include "blue_mob.h"
#include "moon.h"
#include "options.h"
#include "storage.h"
#include "rng.h"
#include "dropped_item.h"

BlueMob::BlueMob(BlueMobData data)
{
    id_ = data.id;
    type_ = EntityType::BLUE_MOB;
    position_ = data.position;
    prev_position_ = data.position;
    next_position_ = data.position;
    yaw_ = data.yaw;
    roll_ = 0;
    health_ = data.health;
    stolen_item_ = data.stolen_item;
    aabb_.center = data.position;
    aabb_.extents = {0.5f, 1.5f, 0.25f};

    //
    // Leg mesh
    //

    glm::vec2 leg_bl = {0.0f, 0.0f};
    glm::vec2 leg_tr = {0.25f, 0.3f};

    float leg_vertices[] = {
        // Front
        -1.0f, -2.0f, 1.0f, leg_bl.x, leg_bl.y,
         1.0f, -2.0f, 1.0f, leg_tr.x, leg_bl.y,
         1.0f,  0.0f, 1.0f, leg_tr.x, leg_tr.y,
         1.0f,  0.0f, 1.0f, leg_tr.x, leg_tr.y,
        -1.0f,  0.0f, 1.0f, leg_bl.x, leg_tr.y,
        -1.0f, -2.0f, 1.0f, leg_bl.x, leg_bl.y,

        // Back
         1.0f,  0.0f, -1.0f, leg_tr.x, leg_tr.y,
         1.0f, -2.0f, -1.0f, leg_tr.x, leg_bl.y,
        -1.0f, -2.0f, -1.0f, leg_bl.x, leg_bl.y,
        -1.0f, -2.0f, -1.0f, leg_bl.x, leg_bl.y,
        -1.0f,  0.0f, -1.0f, leg_bl.x, leg_tr.y,
         1.0f,  0.0f, -1.0f, leg_tr.x, leg_tr.y,

        // Left
        -1.0f,  0.0f, -1.0f, leg_tr.x, leg_tr.y,
        -1.0f, -2.0f, -1.0f, leg_tr.x, leg_bl.y,
        -1.0f, -2.0f,  1.0f, leg_bl.x, leg_bl.y,
        -1.0f, -2.0f,  1.0f, leg_bl.x, leg_bl.y,
        -1.0f,  0.0f,  1.0f, leg_bl.x, leg_tr.y,
        -1.0f,  0.0f, -1.0f, leg_tr.x, leg_tr.y,

        // Right
        1.0f, -2.0f,  1.0f, leg_bl.x, leg_bl.y,
        1.0f, -2.0f, -1.0f, leg_tr.x, leg_bl.y,
        1.0f,  0.0f, -1.0f, leg_tr.x, leg_tr.y,
        1.0f,  0.0f, -1.0f, leg_tr.x, leg_tr.y,
        1.0f,  0.0f,  1.0f, leg_bl.x, leg_tr.y,
        1.0f, -2.0f,  1.0f, leg_bl.x, leg_bl.y,

        // Top
         1.0f, 0.0f,  1.0f, leg_tr.x, leg_tr.y,
         1.0f, 0.0f, -1.0f, leg_tr.x, leg_bl.y,
        -1.0f, 0.0f, -1.0f, leg_bl.x, leg_bl.y,
        -1.0f, 0.0f, -1.0f, leg_bl.x, leg_bl.y,
        -1.0f, 0.0f,  1.0f, leg_bl.x, leg_tr.y,
         1.0f, 0.0f,  1.0f, leg_tr.x, leg_tr.y,

        // Bottom
        -1.0f, -2.0f, -1.0f, leg_bl.x, leg_bl.y,
         1.0f, -2.0f, -1.0f, leg_tr.x, leg_bl.y,
         1.0f, -2.0f,  1.0f, leg_tr.x, leg_tr.y,
         1.0f, -2.0f,  1.0f, leg_tr.x, leg_tr.y,
        -1.0f, -2.0f,  1.0f, leg_bl.x, leg_tr.y,
        -1.0f, -2.0f, -1.0f, leg_bl.x, leg_bl.y,
    };
    leg_mesh_.SetShader(ShaderManager::MOB_SHADER);
    leg_mesh_.SetTexture(Storage::IMAGES / "entities" / "blue_mob.png");
    leg_mesh_.SetVertexData(leg_vertices, sizeof(leg_vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);

    //
    // Body mesh
    //

    glm::vec2 body_front_bl = {0.0f, 0.3f};
    glm::vec2 body_front_tr = {0.5f, 1.0f};

    glm::vec2 body_back_bl = {0.5f, 0.0f};
    glm::vec2 body_back_tr = {1.0f, 2.0f / 3.0f};

    glm::vec2 body_side_bl = {0.5f, 0.0f};
    glm::vec2 body_side_tr = {0.75f, 2.0f / 3.0f};

    float body_vertices[] = {
        // Front
        -1.0f, -1.0f, 1.0f, body_front_bl.x, body_front_bl.y,
         1.0f, -1.0f, 1.0f, body_front_tr.x, body_front_bl.y,
         1.0f,  1.0f, 1.0f, body_front_tr.x, body_front_tr.y,
         1.0f,  1.0f, 1.0f, body_front_tr.x, body_front_tr.y,
        -1.0f,  1.0f, 1.0f, body_front_bl.x, body_front_tr.y,
        -1.0f, -1.0f, 1.0f, body_front_bl.x, body_front_bl.y,

        // Back
         1.0f,  1.0f, -1.0f, body_back_tr.x, body_back_tr.y,
         1.0f, -1.0f, -1.0f, body_back_tr.x, body_back_bl.y,
        -1.0f, -1.0f, -1.0f, body_back_bl.x, body_back_bl.y,
        -1.0f, -1.0f, -1.0f, body_back_bl.x, body_back_bl.y,
        -1.0f,  1.0f, -1.0f, body_back_bl.x, body_back_tr.y,
         1.0f,  1.0f, -1.0f, body_back_tr.x, body_back_tr.y,

        // Left
        -1.0f,  1.0f, -1.0f, body_side_tr.x, body_side_tr.y,
        -1.0f, -1.0f, -1.0f, body_side_tr.x, body_side_bl.y,
        -1.0f, -1.0f,  1.0f, body_side_bl.x, body_side_bl.y,
        -1.0f, -1.0f,  1.0f, body_side_bl.x, body_side_bl.y,
        -1.0f,  1.0f,  1.0f, body_side_bl.x, body_side_tr.y,
        -1.0f,  1.0f, -1.0f, body_side_tr.x, body_side_tr.y,

        // Right
        1.0f, -1.0f,  1.0f, body_side_bl.x, body_side_bl.y,
        1.0f, -1.0f, -1.0f, body_side_tr.x, body_side_bl.y,
        1.0f,  1.0f, -1.0f, body_side_tr.x, body_side_tr.y,
        1.0f,  1.0f, -1.0f, body_side_tr.x, body_side_tr.y,
        1.0f,  1.0f,  1.0f, body_side_bl.x, body_side_tr.y,
        1.0f, -1.0f,  1.0f, body_side_bl.x, body_side_bl.y,

        // Top
         1.0f, 1.0f,  1.0f, body_back_tr.x, body_back_tr.y * (4.0f / 18.0f),
         1.0f, 1.0f, -1.0f, body_back_tr.x, body_back_bl.y,
        -1.0f, 1.0f, -1.0f, body_back_bl.x, body_back_bl.y,
        -1.0f, 1.0f, -1.0f, body_back_bl.x, body_back_bl.y,
        -1.0f, 1.0f,  1.0f, body_back_bl.x, body_back_tr.y * (4.0f / 18.0f),
         1.0f, 1.0f,  1.0f, body_back_tr.x, body_back_tr.y * (4.0f / 18.0f),

        // Bottom
        -1.0f, -1.0f, -1.0f, body_back_bl.x, body_back_bl.y,
         1.0f, -1.0f, -1.0f, body_back_tr.x, body_back_bl.y,
         1.0f, -1.0f,  1.0f, body_back_tr.x, body_back_tr.y * (4.0f / 18.0f),
         1.0f, -1.0f,  1.0f, body_back_tr.x, body_back_tr.y * (4.0f / 18.0f),
        -1.0f, -1.0f,  1.0f, body_back_bl.x, body_back_tr.y * (4.0f / 18.0f),
        -1.0f, -1.0f, -1.0f, body_back_bl.x, body_back_bl.y,
    };
    body_mesh_.SetShader(ShaderManager::MOB_SHADER);
    body_mesh_.SetTexture(Storage::IMAGES / "entities" / "blue_mob.png");
    body_mesh_.SetVertexData(body_vertices, sizeof(body_vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);
}

void BlueMob::Update(float delta_time)
{
    internal_time_ += delta_time;

    if (pain_time_ != 0)
    {
        pain_time_ -= delta_time;
        if (pain_time_ < 0)
            pain_time_ = 0;
    }

    if (IsDead())
    {
        action_ = BlueMobAction::NONE;
        time_since_death_ += delta_time;
    }
    else if (internal_time_ > next_action_time_)
    {
        if (action_ == BlueMobAction::NONE)
        {
            float chance = RNG{}.Range(0.0f, 1.0f);
            if (chance < 0.3f)
            {
                action_ = BlueMobAction::WALK;
                next_action_time_ += RNG{}.Range(5.0f, 10.0f);

                walk_velocity_ = glm::vec3{glm::sin(yaw_), 0, glm::cos(yaw_)};

                if (!Moon::GetCurrentMoon()->GetSettings().is_creative && stolen_item_.IsEmpty())
                {
                    auto player_pos = Moon::GetCurrentMoon()->GetPlayer()->GetPosition();
                    if (glm::length(player_pos - position_) < 25.0f)
                        action_ = BlueMobAction::CHASE;
                }
            }
            else if (chance < 0.5f)
            {
                action_ = BlueMobAction::ROTATE_LEFT;
                next_action_time_ += RNG{}.Range(0.0f, 3.0f);
            }
            else if (chance < 0.7f)
            {
                action_ = BlueMobAction::ROTATE_RIGHT;
                next_action_time_ += RNG{}.Range(0.0f, 3.0f);
            }
            else if (chance < 0.9f)
            {
                action_ = BlueMobAction::WALK;
                next_action_time_ += RNG{}.Range(1.0f, 6.0f);

                walk_velocity_ = glm::vec3{glm::sin(yaw_), 0, glm::cos(yaw_)};
                velocity_.y += 4.0f;
            }
            else
            {
                next_action_time_ += RNG{}.Range(0.0f, 3.0f);
            }
        }
        else
        {
            action_ = BlueMobAction::NONE;
            next_action_time_ += RNG{}.Range(2.0f, 13.0f);
            time_walking_ = 0;
            velocity_.x = 0;
            velocity_.z = 0;
        }
    }
}

void BlueMob::FixedUpdate()
{
    if (IsDead())
    {
        velocity_.x = 0;
        velocity_.z = 0;

        if (!stolen_item_.IsEmpty())
        {
            DroppedItem *dropped = new DroppedItem({
                .position = position_,
                .item = stolen_item_.item,
                .amount = stolen_item_.amount
            });
            dropped->SetVelocity({
                RNG{}.Range(-2.0f, 2.0f),
                RNG{}.Range(2.0f, 4.0f),
                RNG{}.Range(-2.0f, 2.0f)
            });
            Moon::GetCurrentMoon()->GetEntityManager().AddEntity(dropped);
            stolen_item_ = {ItemID::none, 0};
        }

        if (!death_animation_done_)
        {
            roll_ = glm::clamp(90.0f * time_since_death_, 0.0f, 90.0f); // 90 degrees/sec
            if (time_since_death_ > 4.0f)
                death_animation_done_ = true;
        }
    }
    else if (action_ == BlueMobAction::ROTATE_LEFT)
    {
        yaw_ -= glm::radians(25.0f) * FIXED_DELTA_TIME;
    }
    else if (action_ == BlueMobAction::ROTATE_RIGHT)
    {
        yaw_ += glm::radians(25.0f) * FIXED_DELTA_TIME;
    }
    else if (action_ == BlueMobAction::WALK)
    {
        time_walking_ += FIXED_DELTA_TIME;
        velocity_.x = walk_velocity_.x;
        velocity_.z = walk_velocity_.z;
    }
    else if (action_ == BlueMobAction::CHASE)
    {
        auto player = Moon::GetCurrentMoon()->GetPlayer();
        auto player_pos = player->GetPosition();
        auto displacement = player_pos - position_;
        displacement.y = 0;
        float distance = glm::length(displacement);
        displacement = glm::normalize(displacement);

        if (distance < 0.8f)
        {
            auto &player_inventory = player->GetInventory();
            auto &item_to_steal = player_inventory.inventory[0][RNG{}.Range(0, 9)];
            stolen_item_ = item_to_steal;
            item_to_steal = {ItemID::none, 0};
            action_ = BlueMobAction::NONE;

            float tp_dir = RNG{}.Range(0.0f, 2.0f * glm::pi<float>());
            float tp_dist = RNG{}.Range(1.5f * CHUNK_SIZE, 2.5f * CHUNK_SIZE);
            position_ += glm::vec3{tp_dist * glm::cos(tp_dir), 40, tp_dist * glm::sin(tp_dir)};
            prev_position_ = position_;
            next_position_ = position_;

            Moon::GetCurrentMoon()->DisplayMessage("An item was stolen!");
            SoundSystem::Play(SoundSystem::Sound::TELEPORT);

            return;
        }

        velocity_.x = 4.0f * displacement.x;
        velocity_.z = 4.0f * displacement.z;
        
        target_yaw_ = 0;
        if (displacement.x < 0)
            displacement.z *= -1;
        yaw_ = glm::acos(glm::dot(displacement, glm::vec3{0, 0, 1}));
        if (displacement.x < 0)
            yaw_ += glm::pi<float>();

        time_walking_ += FIXED_DELTA_TIME;
    }
}

void BlueMob::Render(const glm::mat4 &view, const glm::mat4 &proj)
{
    auto RotateAround = [](glm::vec3 pivot, glm::quat rot)
    {
        return glm::translate(glm::mat4(1.0f), pivot) *
            glm::mat4_cast(rot) *
            glm::translate(glm::mat4(1.0f), -pivot);
    };

    glm::vec4 fog_color = Moon::GetCurrentMoon()->GetFogColor();
    float render_distance = OptionsManager::GetOptions().render_distance;

    auto &shader = ShaderManager::MOB_SHADER;
    shader.Use();
    shader.SetMat4("u_view", view);
    shader.SetMat4("u_proj", proj);
    shader.SetVec4("u_fog_color", fog_color);
    shader.SetFloat("u_fog_start", (float)render_distance * 0.33f * 32.0f);
    shader.SetFloat("u_fog_end", (float)render_distance * 0.85f * 32.0f);
    shader.SetVec4("u_color", {1.0f, 1.0f - pain_time_, 1.0f - pain_time_, 1.0f});

    if (action_ == BlueMobAction::CHASE || action_ == BlueMobAction::WALK || glm::abs(walk_spread_) > 0.02f)
    {
        float speed = (action_ == BlueMobAction::CHASE) ? 6.0f : 3.0f;
        if (action_ == BlueMobAction::CHASE || action_ == BlueMobAction::WALK)
            walk_spread_ = glm::radians(22.5f * glm::sin(speed * time_walking_));
        else
            walk_spread_ *= 0.99f;
    }
    else
    {
        walk_spread_ = 0;
    }

    glm::mat4 root_transform =
        glm::translate(glm::mat4(1.0f), position_) *
        glm::mat4_cast(glm::angleAxis(yaw_, glm::vec3(0, 1, 0)));

    // Right leg
    glm::mat4 right_leg = root_transform;
    right_leg = glm::translate(right_leg, glm::vec3{-0.25f, -0.5f, 0.0f});
    right_leg *= RotateAround(
        {0.25f, -1.0f, 0.25f},
        glm::angleAxis(glm::radians(roll_), glm::vec3(1, 0, 0))
    );
    right_leg *= glm::mat4_cast(glm::angleAxis(walk_spread_, glm::vec3(1, 0, 0)));
    right_leg = glm::scale(right_leg, glm::vec3{0.25f, 0.5f, 0.25f});
    leg_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", right_leg);
    });

    // Left leg
    glm::mat4 left_leg = root_transform;
    left_leg = glm::translate(left_leg, glm::vec3{0.25f, -0.5f, 0.0f});
    left_leg *= RotateAround(
        {-0.25f, -1.0f, 0.25f},
        glm::angleAxis(glm::radians(roll_), glm::vec3(1, 0, 0))
    );
    left_leg *= glm::mat4_cast(glm::angleAxis(walk_spread_, glm::vec3(-1, 0, 0)));
    left_leg = glm::scale(left_leg, glm::vec3{0.25f, 0.5f, 0.25f});
    leg_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", left_leg);
    });

    // Body
    glm::mat4 body = root_transform;
    body = glm::translate(body, {0, 0.5f, 0});
    body *= RotateAround(
        {0, -2.0f, 0.25f},
        glm::angleAxis(glm::radians(roll_), glm::vec3(1, 0, 0))
    );
    body = glm::scale(body, {0.5f, 1.0f, 0.25f});
    body_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", body);
    });
}

BlueMobData BlueMob::GetBlueMobData()
{
    return {
        .id = id_,
        .position = position_,
        .stolen_item = stolen_item_,
        .yaw = yaw_,
        .health = health_
    };
}

void BlueMob::NotifyOfAttacker(size_t id)
{
    if (id == Moon::GetCurrentMoon()->GetPlayer()->GetID())
        action_ = BlueMobAction::CHASE;
}
