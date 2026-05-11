
#include "minilight.h"
#include "block.h"
#include "storage.h"

Minilight::Minilight(MinilightData data)
{
    id_ = data.id;
    type_ = EntityType::MINILIGHT;
    voxel_ = data.voxel;
    position_ = glm::vec3{data.voxel};
    normal_ = data.normal;
    can_be_damaged_ = false;

    // These are extents, not total lengths
    const float width = 1.0f / 6.0f;
    const float thickness = 1.0f / 64.0f;

    glm::vec3 voxel_center = glm::vec3{data.voxel};

    glm::vec3 v_local_positions[] = {
        // Front
        {-width, -width, thickness},
        { width, -width, thickness},
        { width,  width, thickness},
        { width,  width, thickness},
        {-width,  width, thickness},
        {-width, -width, thickness},

        // Left
        {-width,  width, -thickness},
        {-width, -width, -thickness},
        {-width, -width,  thickness},
        {-width, -width,  thickness},
        {-width,  width,  thickness},
        {-width,  width, -thickness},

        // Right
        {width, -width,  thickness},
        {width, -width, -thickness},
        {width,  width, -thickness},
        {width,  width, -thickness},
        {width,  width,  thickness},
        {width, -width,  thickness},

        // Top
        { width, width,  thickness},
        { width, width, -thickness},
        {-width, width, -thickness},
        {-width, width, -thickness},
        {-width, width,  thickness},
        { width, width,  thickness},

        // Bottom
        {-width, -width, -thickness},
        { width, -width, -thickness},
        { width, -width,  thickness},
        { width, -width,  thickness},
        {-width, -width,  thickness},
        {-width, -width, -thickness},
    };

    float uv_size = 1.0f / 14.0f;
    glm::vec2 tile_origin = GetAtlasTileOrigins()[BlockID::light][0];
    glm::vec2 v_uv[] = {
        {0,       0},
        {uv_size, 0},
        {uv_size, uv_size},
        {uv_size, uv_size},
        {0,       uv_size},
        {0,       0},
    };

    glm::mat4 model{1.0f};
    model = glm::translate(model, voxel_center - (0.5f - thickness) * data.normal);
    if (normal_.x > 0)
        model = glm::rotate(model, glm::radians(90.0f), {0, 1, 0});
    else if (normal_.x < 0)
        model = glm::rotate(model, glm::radians(-90.0f), {0, 1, 0});
    else if (normal_.z < 0)
        model = glm::rotate(model, glm::radians(180.0f), {0, 1, 0});
    else if (normal_.y > 0)
        model = glm::rotate(model, glm::radians(-90.0f), {1, 0, 0});
    else if (normal_.y < 0)
        model = glm::rotate(model, glm::radians(90.0f), {1, 0, 0});

    std::vector<float> vertices;
    int uv_idx = 0;
    for (auto local_pos : v_local_positions)
    {
        glm::vec3 global_pos = glm::vec3{model * glm::vec4{local_pos, 1.0f}};
        glm::vec2 uv = v_uv[uv_idx];

        vertices.push_back(global_pos.x);
        vertices.push_back(global_pos.y);
        vertices.push_back(global_pos.z);
        vertices.push_back(tile_origin.x + uv.x);
        vertices.push_back(tile_origin.y + uv.y);

        uv_idx = (uv_idx + 1) % 6;
    }

    mesh_.SetShader(ShaderManager::SIMPLE_UNLIT_SHADER);
    mesh_.SetTexture(Storage::IMAGES / "texture_atlas.png", GL_NEAREST);
    mesh_.SetVertexData(vertices.data(), 30, GL_STATIC_DRAW);
}

void Minilight::Render(const glm::mat4 &view, const glm::mat4 &proj)
{
    glm::mat4 model{ 1.0f };

    mesh_.Render([&](Shader* shader) {
        shader->SetMat4("u_model", model);
        shader->SetMat4("u_view", view);
        shader->SetMat4("u_proj", proj);
    });
}

glm::ivec3 Minilight::GetVoxel()
{
    return voxel_;
}

MinilightData Minilight::GetMinilightData()
{
    return {
        .id = id_,
        .voxel = voxel_,
        .normal = normal_
    };
}

void Minilight::Update(float delta_time) {}
void Minilight::FixedUpdate() {}
