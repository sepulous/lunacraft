
#include "green_mob.h"
#include "moon.h"
#include "options.h"
#include "storage.h"
#include "rng.h"

GreenMob::GreenMob(GreenMobData data)
{
    type_ = EntityType::GREEN_MOB;
    position_ = data.position;
    prev_position_ = data.position;
    next_position_ = data.position;
    rotation_ = data.rotation;
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
    if (is_grounded_)
    {
        if (next_action_time_ <= 0)
        {
            next_action_time_ = RNG{}.Range(1.0f, 5.0f);
            if (RNG{}.Range(0, 1) == 0)
            {
                action_ = GreenMobAction::JUMP;

                auto yaw_rotation = glm::mat3{glm::rotate(glm::mat4{1.0f}, glm::radians(rotation_), {0, 1, 0})};
                glm::vec3 forward = yaw_rotation * glm::vec3{0.0f, 0.0f, 1.0f};
                jump_vector_ = RNG{}.Range(1.0f, 6.0f) * forward
                             + RNG{}.Range(1.0f, 6.0f) * glm::vec3{0, 1, 0};
            }
            else
            {
                action_ = GreenMobAction::ROTATE;

                rotation_angle_ = RNG{}.Range(10.0f, 90.0f);
                if (RNG{}.Range(0, 1) == 0)
                    rotation_angle_ *= -1;
            }
        }
        else if (action_ == GreenMobAction::NONE)
        {
            next_action_time_ -= delta_time;
        }
    }
}

void GreenMob::FixedUpdate()
{
    if (action_ == GreenMobAction::JUMP)
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
                action_ = GreenMobAction::NONE;
            }
        }
    }
    else if (action_ == GreenMobAction::ROTATE)
    {
        if (rotation_angle_ > 0)
        {
            float delta = 80.0f * FIXED_DELTA_TIME;
            rotation_ += delta;
            rotation_angle_ -= delta;
        }
        else
        {
            action_ = GreenMobAction::NONE;
        }
    }
}

void GreenMob::Render(const glm::mat4 &vp_matrix)
{
    glm::vec3 camera_pos = Moon::GetCurrentMoon()->GetPlayer()->GetCamera().position;
    glm::vec4 fog_color = Moon::GetCurrentMoon()->GetFogColor();
    float fog_distance = OptionsManager::GetOptions().render_distance * (CHUNK_SIZE / 1.5f);

    glm::mat4 model{1.0f};
    model = glm::translate(model, position_);
    model = glm::rotate(model, glm::radians(rotation_), {0, 1, 0});
    model = glm::scale(model, glm::vec3{0.5f});

    mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_vp_matrix", vp_matrix);
        shader->SetMat4("u_model_matrix", model);
        shader->SetVec3("u_ws_camera_position", camera_pos);
        shader->SetVec4("u_fog_color", fog_color);
        shader->SetFloat("u_fog_distance", fog_distance);
    });
}

GreenMobData GreenMob::GetGreenMobData()
{
    return {
        .position = position_,
        .rotation = rotation_,
        .health = health_
    };
}
