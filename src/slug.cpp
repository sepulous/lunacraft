
#include "slug.h"
#include "storage.h"
#include "constants.h"

Slug::Slug(SlugData slug_data)
{
    type_ = EntityType::SLUG;
    position_ = slug_data.initial_position;
    prev_position_ = slug_data.initial_position;
    next_position_ = slug_data.initial_position;
    initial_position_ = slug_data.initial_position;
    velocity_ = slug_data.initial_velocity;
    initial_velocity_ = slug_data.initial_velocity;
    time_flying_ = slug_data.time_flying;
    damage_ = slug_data.damage;
    is_flying_ = slug_data.is_flying;
    time_stuck_ = slug_data.time_stuck;
    can_be_damaged_ = false;
    death_animation_done_ = true;

    float vertices[] = {
        // Left
        -1.0f,  1.0f,  3.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, -3.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -3.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -3.0f, 0.0f, 0.0f,
        -1.0f, -1.0f,  3.0f, 0.0f, 1.0f,
        -1.0f,  1.0f,  3.0f, 1.0f, 1.0f,

        // Right
        1.0f, -1.0f, -3.0f, 0.0f, 0.0f,
        1.0f,  1.0f, -3.0f, 1.0f, 0.0f,
        1.0f,  1.0f,  3.0f, 1.0f, 1.0f,
        1.0f,  1.0f,  3.0f, 1.0f, 1.0f,
        1.0f, -1.0f,  3.0f, 0.0f, 1.0f,
        1.0f, -1.0f, -3.0f, 0.0f, 0.0f,

        // Top
         1.0f, 1.0f,  3.0f, 1.0f, 1.0f,
         1.0f, 1.0f, -3.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, -3.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, -3.0f, 0.0f, 0.0f,
        -1.0f, 1.0f,  3.0f, 0.0f, 1.0f,
         1.0f, 1.0f,  3.0f, 1.0f, 1.0f,

        // Bottom
        -1.0f, -1.0f, -3.0f, 0.0f, 0.0f,
         1.0f, -1.0f, -3.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  3.0f, 1.0f, 1.0f,
         1.0f, -1.0f,  3.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  3.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -3.0f, 0.0f, 0.0f,

        // Front
        -1.0f, -1.0f, 3.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 3.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 3.0f, 1.0f, 0.3f,
         1.0f,  1.0f, 3.0f, 1.0f, 0.3f,
        -1.0f,  1.0f, 3.0f, 0.0f, 0.3f,
        -1.0f, -1.0f, 3.0f, 0.0f, 0.0f,

        // Back
         1.0f,  1.0f, -3.0f, 1.0f, 0.3f,
         1.0f, -1.0f, -3.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -3.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -3.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, -3.0f, 0.0f, 0.3f,
         1.0f,  1.0f, -3.0f, 1.0f, 0.3f,
    };

    mesh_.SetShader(ShaderManager::SIMPLE_UNLIT_SHADER);
    mesh_.SetTexture(Storage::IMAGES / "entities" / "slug.png");
    mesh_.SetVertexData(vertices, sizeof(vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);
}

void Slug::Update(float delta_time)
{
    if (is_flying_)
        time_flying_ += delta_time;
    else
        time_stuck_ += delta_time;

    if (time_stuck_ > 15.0f)
        SetIsDead(true);
}

void Slug::FixedUpdate()
{
    if (is_flying_)
    {
        velocity_ = {
            initial_velocity_.x,
            initial_velocity_.y - PLAYER_GRAVITY * time_flying_,
            initial_velocity_.z
        };
    }
}

void Slug::Render(const glm::mat4 &vp_matrix)
{
    glm::vec3 forward = glm::normalize(velocity_);
    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));
    glm::vec3 up = glm::cross(forward, right);
    glm::mat4 rotation = glm::mat4(
        glm::vec4(right,   0.0f),
        glm::vec4(up,      0.0f),
        glm::vec4(forward, 0.0f),
        glm::vec4(0.0f, 0.0f , 0.0f, 1.0f)
    );

    glm::mat4 model = glm::translate(glm::mat4{1.0f}, position_) * rotation;
    model = glm::scale(model, glm::vec3{0.05f});

    mesh_.Render([&model, &vp_matrix](Shader *shader) {
        shader->SetMat4("u_mvp_matrix", vp_matrix * model);
    });
}

SlugData Slug::GetSlugData()
{
    return {
        .initial_position = position_,
        .initial_velocity = velocity_,
        .source_id = source_id_,
        .time_flying = time_flying_,
        .time_stuck = time_stuck_,
        .damage = damage_,
        .is_flying = is_flying_
    };
}

void Slug::SetIsFlying(bool value)
{
    is_flying_ = value;
}

bool Slug::IsFlying()
{
    return is_flying_;
}
