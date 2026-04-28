
#include "astronaut.h"
#include "moon.h"
#include "options.h"
#include "storage.h"
#include "rng.h"
#include "slug.h"
#include "sound_system.h"
#include "dropped_item.h"

Astronaut::Astronaut(AstronautData data)
{
    id_ = data.id;
    type_ = EntityType::ASTRONAUT;
    position_ = data.position;
    prev_position_ = data.position;
    next_position_ = data.position;
    position_last_frame_ = data.position;
    velocity_ = data.velocity;
    move_velocity_ = data.move_velocity;
    yaw_ = data.yaw;
    health_ = data.health;
    level_ = data.level;
    target_entity_id_ = data.target_entity_id;
    action_ = static_cast<AstronautAction>(data.action);
    friendly_ = data.friendly;
    aabb_.center = data.position;
    aabb_.extents = {0.5f, 1.64f, 0.5f};

    //
    // Body mesh
    //

    glm::vec2 body_front_size = {8.0f / 24.0f, 12.0f / 28.0f};
    glm::vec2 body_front_bl = {8.0f / 24.0f, 0.0f / 28.0f};
    glm::vec2 body_front_tr = body_front_bl + body_front_size;

    glm::vec2 body_side_size = {8.0f / 24.0f, 12.0f / 28.0f};
    glm::vec2 body_side_bl = {16.0f / 24.0f, 0.0f / 28.0f};
    glm::vec2 body_side_tr = body_side_bl + body_side_size;

    float body_vertices[] = {
        // Front
        -1.0f, -1.0f, 1.0f, body_front_bl.x, body_front_bl.y,
         1.0f, -1.0f, 1.0f, body_front_tr.x, body_front_bl.y,
         1.0f,  1.0f, 1.0f, body_front_tr.x, body_front_tr.y,
         1.0f,  1.0f, 1.0f, body_front_tr.x, body_front_tr.y,
        -1.0f,  1.0f, 1.0f, body_front_bl.x, body_front_tr.y,
        -1.0f, -1.0f, 1.0f, body_front_bl.x, body_front_bl.y,

        // Back
         1.0f,  1.0f, -1.0f, body_side_tr.x, body_side_tr.y,
         1.0f, -1.0f, -1.0f, body_side_tr.x, body_side_bl.y,
        -1.0f, -1.0f, -1.0f, body_side_bl.x, body_side_bl.y,
        -1.0f, -1.0f, -1.0f, body_side_bl.x, body_side_bl.y,
        -1.0f,  1.0f, -1.0f, body_side_bl.x, body_side_tr.y,
         1.0f,  1.0f, -1.0f, body_side_tr.x, body_side_tr.y,

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
         1.0f, 1.0f,  1.0f, body_side_tr.x, body_side_tr.y,
         1.0f, 1.0f, -1.0f, body_side_tr.x, body_side_bl.y,
        -1.0f, 1.0f, -1.0f, body_side_bl.x, body_side_bl.y,
        -1.0f, 1.0f, -1.0f, body_side_bl.x, body_side_bl.y,
        -1.0f, 1.0f,  1.0f, body_side_bl.x, body_side_tr.y,
         1.0f, 1.0f,  1.0f, body_side_tr.x, body_side_tr.y,

        // Bottom
        -1.0f, -1.0f, -1.0f, body_side_bl.x, body_side_bl.y,
         1.0f, -1.0f, -1.0f, body_side_tr.x, body_side_bl.y,
         1.0f, -1.0f,  1.0f, body_side_tr.x, body_side_tr.y,
         1.0f, -1.0f,  1.0f, body_side_tr.x, body_side_tr.y,
        -1.0f, -1.0f,  1.0f, body_side_bl.x, body_side_tr.y,
        -1.0f, -1.0f, -1.0f, body_side_bl.x, body_side_bl.y,
    };
    body_mesh_.SetShader(ShaderManager::MOB_SHADER);
    body_mesh_.SetTexture(Storage::IMAGES / "entities" / "astronaut.png");
    body_mesh_.SetVertexData(body_vertices, sizeof(body_vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);

    //
    // Limb mesh
    //

    glm::vec2 limb_size = {2.0f / 24.0f, 4.0f / 28.0f};
    glm::vec2 limb_bl = {19.0f / 24.0f, 7.0f / 28.0f};
    glm::vec2 limb_tr = limb_bl + limb_size;

    float limb_vertices[] = {
        // Front
        -1.0f, -1.0f, 1.0f, limb_bl.x, limb_bl.y,
         1.0f, -1.0f, 1.0f, limb_tr.x, limb_bl.y,
         1.0f,  1.0f, 1.0f, limb_tr.x, limb_tr.y,
         1.0f,  1.0f, 1.0f, limb_tr.x, limb_tr.y,
        -1.0f,  1.0f, 1.0f, limb_bl.x, limb_tr.y,
        -1.0f, -1.0f, 1.0f, limb_bl.x, limb_bl.y,

        // Back
         1.0f,  1.0f, -1.0f, limb_tr.x, limb_tr.y,
         1.0f, -1.0f, -1.0f, limb_tr.x, limb_bl.y,
        -1.0f, -1.0f, -1.0f, limb_bl.x, limb_bl.y,
        -1.0f, -1.0f, -1.0f, limb_bl.x, limb_bl.y,
        -1.0f,  1.0f, -1.0f, limb_bl.x, limb_tr.y,
         1.0f,  1.0f, -1.0f, limb_tr.x, limb_tr.y,

        // Left
        -1.0f,  1.0f, -1.0f, limb_tr.x, limb_tr.y,
        -1.0f, -1.0f, -1.0f, limb_tr.x, limb_bl.y,
        -1.0f, -1.0f,  1.0f, limb_bl.x, limb_bl.y,
        -1.0f, -1.0f,  1.0f, limb_bl.x, limb_bl.y,
        -1.0f,  1.0f,  1.0f, limb_bl.x, limb_tr.y,
        -1.0f,  1.0f, -1.0f, limb_tr.x, limb_tr.y,

        // Right
        1.0f, -1.0f,  1.0f, limb_bl.x, limb_bl.y,
        1.0f, -1.0f, -1.0f, limb_tr.x, limb_bl.y,
        1.0f,  1.0f, -1.0f, limb_tr.x, limb_tr.y,
        1.0f,  1.0f, -1.0f, limb_tr.x, limb_tr.y,
        1.0f,  1.0f,  1.0f, limb_bl.x, limb_tr.y,
        1.0f, -1.0f,  1.0f, limb_bl.x, limb_bl.y,

        // Top
         1.0f, 1.0f,  1.0f, limb_tr.x, limb_tr.y,
         1.0f, 1.0f, -1.0f, limb_tr.x, limb_bl.y,
        -1.0f, 1.0f, -1.0f, limb_bl.x, limb_bl.y,
        -1.0f, 1.0f, -1.0f, limb_bl.x, limb_bl.y,
        -1.0f, 1.0f,  1.0f, limb_bl.x, limb_tr.y,
         1.0f, 1.0f,  1.0f, limb_tr.x, limb_tr.y,

        // Bottom
        -1.0f, -1.0f, -1.0f, limb_bl.x, limb_bl.y,
         1.0f, -1.0f, -1.0f, limb_tr.x, limb_bl.y,
         1.0f, -1.0f,  1.0f, limb_tr.x, limb_tr.y,
         1.0f, -1.0f,  1.0f, limb_tr.x, limb_tr.y,
        -1.0f, -1.0f,  1.0f, limb_bl.x, limb_tr.y,
        -1.0f, -1.0f, -1.0f, limb_bl.x, limb_bl.y,
    };
    limb_mesh_.SetShader(ShaderManager::MOB_SHADER);
    limb_mesh_.SetTexture(Storage::IMAGES / "entities" / "astronaut.png");
    limb_mesh_.SetVertexData(limb_vertices, sizeof(limb_vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);

    //
    // Head mesh
    //

    glm::vec2 head_front_size = {8.0f / 24.0f, 8.0f / 28.0f};
    glm::vec2 head_front_bl = {8.0f / 24.0f, 12.0f / 28.0f};
    glm::vec2 head_front_tr = head_front_bl + head_front_size;

    glm::vec2 head_back_size = {1.0f / 24.0f, 1.0f / 28.0f};
    glm::vec2 head_back_bl = {0.0f / 24.0f, 13.0f / 28.0f};
    glm::vec2 head_back_tr = head_back_bl + head_back_size;

    glm::vec2 head_top_size = {8.0f / 24.0f, 8.0f / 28.0f};
    glm::vec2 head_top_bl = {8.0f / 24.0f, 20.0f / 28.0f};
    glm::vec2 head_top_tr = head_top_bl + head_top_size;

    glm::vec2 head_left_size = {8.0f / 24.0f, 8.0f / 28.0f};
    glm::vec2 head_left_bl = {16.0f / 24.0f, 12.0f / 28.0f};
    glm::vec2 head_left_tr = head_left_bl + head_left_size;

    glm::vec2 head_right_size = {8.0f / 24.0f, 8.0f / 28.0f};
    glm::vec2 head_right_bl = {0.0f / 24.0f, 12.0f / 28.0f};
    glm::vec2 head_right_tr = head_right_bl + head_right_size;

    float head_vertices[] = {
        // Front
        -1.0f, -1.0f, 1.0f, head_front_bl.x, head_front_bl.y,
         1.0f, -1.0f, 1.0f, head_front_tr.x, head_front_bl.y,
         1.0f,  1.0f, 1.0f, head_front_tr.x, head_front_tr.y,
         1.0f,  1.0f, 1.0f, head_front_tr.x, head_front_tr.y,
        -1.0f,  1.0f, 1.0f, head_front_bl.x, head_front_tr.y,
        -1.0f, -1.0f, 1.0f, head_front_bl.x, head_front_bl.y,

        // Back
         1.0f,  1.0f, -1.0f, head_back_tr.x, head_back_tr.y,
         1.0f, -1.0f, -1.0f, head_back_tr.x, head_back_bl.y,
        -1.0f, -1.0f, -1.0f, head_back_bl.x, head_back_bl.y,
        -1.0f, -1.0f, -1.0f, head_back_bl.x, head_back_bl.y,
        -1.0f,  1.0f, -1.0f, head_back_bl.x, head_back_tr.y,
         1.0f,  1.0f, -1.0f, head_back_tr.x, head_back_tr.y,

        // Left
        1.0f, -1.0f,  1.0f, head_left_bl.x, head_left_bl.y,
        1.0f, -1.0f, -1.0f, head_left_tr.x, head_left_bl.y,
        1.0f,  1.0f, -1.0f, head_left_tr.x, head_left_tr.y,
        1.0f,  1.0f, -1.0f, head_left_tr.x, head_left_tr.y,
        1.0f,  1.0f,  1.0f, head_left_bl.x, head_left_tr.y,
        1.0f, -1.0f,  1.0f, head_left_bl.x, head_left_bl.y,

        // Right
        -1.0f, -1.0f, -1.0f, head_right_bl.x, head_right_bl.y,
        -1.0f, -1.0f,  1.0f, head_right_tr.x, head_right_bl.y,
        -1.0f,  1.0f,  1.0f, head_right_tr.x, head_right_tr.y,
        -1.0f,  1.0f,  1.0f, head_right_tr.x, head_right_tr.y,
        -1.0f,  1.0f, -1.0f, head_right_bl.x, head_right_tr.y,
        -1.0f, -1.0f, -1.0f, head_right_bl.x, head_right_bl.y,

        // Top
         1.0f, 1.0f,  1.0f, head_top_tr.x, head_top_bl.y,
         1.0f, 1.0f, -1.0f, head_top_tr.x, head_top_tr.y,
        -1.0f, 1.0f, -1.0f, head_top_bl.x, head_top_tr.y,
        -1.0f, 1.0f, -1.0f, head_top_bl.x, head_top_tr.y,
        -1.0f, 1.0f,  1.0f, head_top_bl.x, head_top_bl.y,
         1.0f, 1.0f,  1.0f, head_top_tr.x, head_top_bl.y,
    };
    head_mesh_.SetShader(ShaderManager::MOB_SHADER);
    head_mesh_.SetTexture(Storage::IMAGES / "entities" / "astronaut.png");
    head_mesh_.SetVertexData(head_vertices, sizeof(head_vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);

    //
    // Backpack mesh
    //

    glm::vec2 backpack_fb_size = {8.0f / 24.0f, 12.0f / 28.0f};
    glm::vec2 backpack_fb_bl = {0.0f / 24.0f, 0.0f / 28.0f};
    glm::vec2 backpack_fb_tr = backpack_fb_bl + backpack_fb_size;

    glm::vec2 backpack_side_size = {4.0f / 24.0f, 12.0f / 28.0f};
    glm::vec2 backpack_side_bl = {0.0f / 24.0f, 0.0f / 28.0f};
    glm::vec2 backpack_side_tr = backpack_side_bl + backpack_side_size;

    glm::vec2 backpack_tb_size = {2.0f / 24.0f, 6.0f / 28.0f};
    glm::vec2 backpack_tb_bl = {0.0f / 24.0f, 0.0f / 28.0f};
    glm::vec2 backpack_tb_tr = backpack_tb_bl + backpack_tb_size;

    float backpack_vertices[] = {
        // Back
        -1.0f, -1.0f, 1.0f, backpack_fb_bl.x, backpack_fb_bl.y,
         1.0f, -1.0f, 1.0f, backpack_fb_tr.x, backpack_fb_bl.y,
         1.0f,  1.0f, 1.0f, backpack_fb_tr.x, backpack_fb_tr.y,
         1.0f,  1.0f, 1.0f, backpack_fb_tr.x, backpack_fb_tr.y,
        -1.0f,  1.0f, 1.0f, backpack_fb_bl.x, backpack_fb_tr.y,
        -1.0f, -1.0f, 1.0f, backpack_fb_bl.x, backpack_fb_bl.y,

        // Front
         1.0f,  1.0f, -1.0f, backpack_fb_tr.x, backpack_fb_tr.y,
         1.0f, -1.0f, -1.0f, backpack_fb_tr.x, backpack_fb_bl.y,
        -1.0f, -1.0f, -1.0f, backpack_fb_bl.x, backpack_fb_bl.y,
        -1.0f, -1.0f, -1.0f, backpack_fb_bl.x, backpack_fb_bl.y,
        -1.0f,  1.0f, -1.0f, backpack_fb_bl.x, backpack_fb_tr.y,
         1.0f,  1.0f, -1.0f, backpack_fb_tr.x, backpack_fb_tr.y,

        // Left
        -1.0f,  1.0f, -1.0f, backpack_side_tr.x, backpack_side_tr.y,
        -1.0f, -1.0f, -1.0f, backpack_side_tr.x, backpack_side_bl.y,
        -1.0f, -1.0f,  1.0f, backpack_side_bl.x, backpack_side_bl.y,
        -1.0f, -1.0f,  1.0f, backpack_side_bl.x, backpack_side_bl.y,
        -1.0f,  1.0f,  1.0f, backpack_side_bl.x, backpack_side_tr.y,
        -1.0f,  1.0f, -1.0f, backpack_side_tr.x, backpack_side_tr.y,

        // Right
        1.0f, -1.0f,  1.0f, backpack_side_bl.x, backpack_side_bl.y,
        1.0f, -1.0f, -1.0f, backpack_side_tr.x, backpack_side_bl.y,
        1.0f,  1.0f, -1.0f, backpack_side_tr.x, backpack_side_tr.y,
        1.0f,  1.0f, -1.0f, backpack_side_tr.x, backpack_side_tr.y,
        1.0f,  1.0f,  1.0f, backpack_side_bl.x, backpack_side_tr.y,
        1.0f, -1.0f,  1.0f, backpack_side_bl.x, backpack_side_bl.y,

        // Top
         1.0f, 1.0f,  1.0f, backpack_tb_tr.x, backpack_tb_tr.y,
         1.0f, 1.0f, -1.0f, backpack_tb_tr.x, backpack_tb_bl.y,
        -1.0f, 1.0f, -1.0f, backpack_tb_bl.x, backpack_tb_bl.y,
        -1.0f, 1.0f, -1.0f, backpack_tb_bl.x, backpack_tb_bl.y,
        -1.0f, 1.0f,  1.0f, backpack_tb_bl.x, backpack_tb_tr.y,
         1.0f, 1.0f,  1.0f, backpack_tb_tr.x, backpack_tb_tr.y,

        // Bottom
        -1.0f, -1.0f, -1.0f, backpack_tb_bl.x, backpack_tb_bl.y,
         1.0f, -1.0f, -1.0f, backpack_tb_tr.x, backpack_tb_bl.y,
         1.0f, -1.0f,  1.0f, backpack_tb_tr.x, backpack_tb_tr.y,
         1.0f, -1.0f,  1.0f, backpack_tb_tr.x, backpack_tb_tr.y,
        -1.0f, -1.0f,  1.0f, backpack_tb_bl.x, backpack_tb_tr.y,
        -1.0f, -1.0f, -1.0f, backpack_tb_bl.x, backpack_tb_bl.y,
    };
    backpack_mesh_.SetShader(ShaderManager::MOB_SHADER);
    backpack_mesh_.SetTexture(Storage::IMAGES / "entities" / "astronaut.png");
    backpack_mesh_.SetVertexData(backpack_vertices, sizeof(backpack_vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);

    //
    // Pistol mesh
    //

    glm::vec2 pistol_front_size = {4.0f / 24.0f, 4.0f / 28.0f};
    glm::vec2 pistol_front_bl = {0.0f / 24.0f, 20.0f / 28.0f};
    glm::vec2 pistol_front_tr = pistol_front_bl + pistol_front_size;

    glm::vec2 pistol_side_light_size = {1.0f / 24.0f, 1.0f / 28.0f};
    glm::vec2 pistol_side_light_bl = {0.0f / 24.0f, 21.0f / 28.0f};
    glm::vec2 pistol_side_light_tr = pistol_side_light_bl + pistol_side_light_size;

    glm::vec2 pistol_side_dark_size = {1.0f / 24.0f, 1.0f / 28.0f};
    glm::vec2 pistol_side_dark_bl = {1.0f / 24.0f, 21.0f / 28.0f};
    glm::vec2 pistol_side_dark_tr = pistol_side_dark_bl + pistol_side_dark_size;

    float pistol_vertices[] = {
        // Front
        -1.0f, -1.0f, 1.0f, pistol_front_bl.x, pistol_front_bl.y,
         1.0f, -1.0f, 1.0f, pistol_front_tr.x, pistol_front_bl.y,
         1.0f,  1.0f, 1.0f, pistol_front_tr.x, pistol_front_tr.y,
         1.0f,  1.0f, 1.0f, pistol_front_tr.x, pistol_front_tr.y,
        -1.0f,  1.0f, 1.0f, pistol_front_bl.x, pistol_front_tr.y,
        -1.0f, -1.0f, 1.0f, pistol_front_bl.x, pistol_front_bl.y,

        // Back
         1.0f,  1.0f, -1.0f, pistol_side_light_tr.x, pistol_side_light_tr.y,
         1.0f, -1.0f, -1.0f, pistol_side_light_tr.x, pistol_side_light_bl.y,
        -1.0f, -1.0f, -1.0f, pistol_side_light_bl.x, pistol_side_light_bl.y,
        -1.0f, -1.0f, -1.0f, pistol_side_light_bl.x, pistol_side_light_bl.y,
        -1.0f,  1.0f, -1.0f, pistol_side_light_bl.x, pistol_side_light_tr.y,
         1.0f,  1.0f, -1.0f, pistol_side_light_tr.x, pistol_side_light_tr.y,

        // Left
        -1.0f,  1.0f, -1.0f, pistol_side_dark_tr.x, pistol_side_dark_tr.y,
        -1.0f, -1.0f, -1.0f, pistol_side_dark_tr.x, pistol_side_dark_bl.y,
        -1.0f, -1.0f,  1.0f, pistol_side_dark_bl.x, pistol_side_dark_bl.y,
        -1.0f, -1.0f,  1.0f, pistol_side_dark_bl.x, pistol_side_dark_bl.y,
        -1.0f,  1.0f,  1.0f, pistol_side_dark_bl.x, pistol_side_dark_tr.y,
        -1.0f,  1.0f, -1.0f, pistol_side_dark_tr.x, pistol_side_dark_tr.y,

        // Right
        1.0f, -1.0f,  1.0f, pistol_side_dark_bl.x, pistol_side_dark_bl.y,
        1.0f, -1.0f, -1.0f, pistol_side_dark_tr.x, pistol_side_dark_bl.y,
        1.0f,  1.0f, -1.0f, pistol_side_dark_tr.x, pistol_side_dark_tr.y,
        1.0f,  1.0f, -1.0f, pistol_side_dark_tr.x, pistol_side_dark_tr.y,
        1.0f,  1.0f,  1.0f, pistol_side_dark_bl.x, pistol_side_dark_tr.y,
        1.0f, -1.0f,  1.0f, pistol_side_dark_bl.x, pistol_side_dark_bl.y,

        // Top
         1.0f, 1.0f,  1.0f, pistol_side_light_tr.x, pistol_side_light_tr.y,
         1.0f, 1.0f, -1.0f, pistol_side_light_tr.x, pistol_side_light_bl.y,
        -1.0f, 1.0f, -1.0f, pistol_side_light_bl.x, pistol_side_light_bl.y,
        -1.0f, 1.0f, -1.0f, pistol_side_light_bl.x, pistol_side_light_bl.y,
        -1.0f, 1.0f,  1.0f, pistol_side_light_bl.x, pistol_side_light_tr.y,
         1.0f, 1.0f,  1.0f, pistol_side_light_tr.x, pistol_side_light_tr.y,

        // Bottom
        -1.0f, -1.0f, -1.0f, pistol_side_light_bl.x, pistol_side_light_bl.y,
         1.0f, -1.0f, -1.0f, pistol_side_light_tr.x, pistol_side_light_bl.y,
         1.0f, -1.0f,  1.0f, pistol_side_light_tr.x, pistol_side_light_tr.y,
         1.0f, -1.0f,  1.0f, pistol_side_light_tr.x, pistol_side_light_tr.y,
        -1.0f, -1.0f,  1.0f, pistol_side_light_bl.x, pistol_side_light_tr.y,
        -1.0f, -1.0f, -1.0f, pistol_side_light_bl.x, pistol_side_light_bl.y,
    };
    pistol_mesh_.SetShader(ShaderManager::MOB_SHADER);
    pistol_mesh_.SetTexture(Storage::IMAGES / "entities" / "astronaut.png");
    pistol_mesh_.SetVertexData(pistol_vertices, sizeof(pistol_vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);
}

void Astronaut::Update(float delta_time)
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
        if (action_ == AstronautAction::NONE)
        {
            bool consumed_gold = Moon::GetCurrentMoon()->GetEntityManager().DestroyItemNear(ItemID::gold, position_, 7.5f);
            if (consumed_gold)
                friendly_ = true;

            float chance = RNG{}.Range(0.0f, 1.0f);
            if (chance < 0.33f)
            {
                action_ = AstronautAction::MOVE;
                next_action_time_ += RNG{}.Range(1.0f, 4.0f);

                move_velocity_ = {10.0f * glm::sin(glm::radians(yaw_)), 0, 10.0f * glm::cos(glm::radians(yaw_))};

                if (friendly_)
                {
                    auto nearest_mob = Moon::GetCurrentMoon()->GetEntityManager().GetNearestMob(GetID(), 0.5f * (level_ * 15 + 30));
                    if (nearest_mob && RNG{}.Range(0.0f, 1.0f) < 0.85f)
                    {
                        if (!(nearest_mob->GetType() == EntityType::ASTRONAUT && dynamic_cast<Astronaut *>(nearest_mob)->friendly_))
                        {
                            target_entity_id_ = nearest_mob->GetID();
                            action_ = AstronautAction::ATTACK;
                        }
                    }
                }
                else
                {
                    auto player = Moon::GetCurrentMoon()->GetPlayer();
                    float player_distance = glm::length(player->GetPosition() - position_);
                    if (false && player_distance < 0.5f * (level_ * 15 + 30))
                    {
                        target_entity_id_ = player->GetID();
                        action_ = AstronautAction::ATTACK;
                    }
                    else
                    {
                        auto nearest_mob = Moon::GetCurrentMoon()->GetEntityManager().GetNearestMob(GetID(), 0.5f * (level_ * 15 + 30));
                        if (nearest_mob && nearest_mob->GetType() != EntityType::ASTRONAUT && RNG{}.Range(0.0f, 1.0f) < 0.5f)
                        {
                            target_entity_id_ = nearest_mob->GetID();
                            action_ = AstronautAction::ATTACK;
                        }
                    }
                }
            }
            else if (chance < 0.5f)
            {
                action_ = AstronautAction::ROTATE_LEFT;
                next_action_time_ += RNG{}.Range(0.0f, 1.0f);
            }
            else if (chance < 0.7f)
            {
                action_ = AstronautAction::ROTATE_RIGHT;
                next_action_time_ += RNG{}.Range(0.0f, 1.0f);
            }
            else if (chance < 0.9f)
            {
                action_ = AstronautAction::MOVE;
                next_action_time_ += RNG{}.Range(1.0f, 4.0f);

                velocity_.y += RNG{}.Range(4.0f, 10.0f);

                move_velocity_ = {1.0f * glm::sin(glm::radians(yaw_)), 0, 1.0f * glm::cos(glm::radians(yaw_))};
            }
        }
        else if (action_ == AstronautAction::ATTACK)
        {
            Entity *target = Moon::GetCurrentMoon()->GetEntityManager().GetEntityByID(target_entity_id_);
            if (!target || target->IsDead())
            {
                action_ = AstronautAction::NONE;
                next_action_time_ += 1.0f;
                return;
            }

            if (RNG{}.Range(0.0f, 1.0f) < 0.1f)
                velocity_.y += RNG{}.Range(2.0f, 8.0f) + level_ * 0.5f;

            float distance = glm::length(target->GetPosition() - position_);

            if (distance > 50.0f)
            {
                action_ = AstronautAction::NONE;
                next_action_time_ += 1.0f;
                return;
            }

            // Move around target
            float move_speed = level_ * 1.0f + 10.0f;
            float move_angle = yaw_ + (distance > 15.0f ? 0 : 180.0 + RNG{}.Range(-45.0f, 45.0f));
            move_velocity_ = {
                move_speed * glm::sin(glm::radians(move_angle)),
                0,
                move_speed * glm::cos(glm::radians(move_angle))
            };

            // Determine direction
            auto displacement = target->GetPosition() - position_;
            float pitch = glm::asin(displacement.y / glm::length(displacement));
            glm::vec3 slug_dir {
                cos(pitch) * sin(glm::radians(yaw_)),
                sin(pitch),
                cos(pitch) * cos(glm::radians(yaw_))
            };

            // Determine velocity
            float slug_speed = RNG{}.Range(20.0f, 20.0f + level_ * 6.0f);
            glm::vec3 slug_velocity = slug_speed * slug_dir;
            slug_velocity.y += 2.0f; // Gravity compensation

            // Fire
            Slug *slug = new Slug({
                .initial_position = position_,
                .initial_velocity = slug_velocity,
                .source_id = GetID(),
                .damage = level_ * 8 + 20
            });
            Moon::GetCurrentMoon()->GetEntityManager().AddEntity(slug);
            SoundSystem::PlayAt(SoundSystem::Sound::LASER, position_);

            next_action_time_ += 3.0f / (level_ + 1) + RNG{}.Range(0.0f, 0.8f);
        }
        else
        {
            action_ = AstronautAction::NONE;
            next_action_time_ += RNG{}.Range(2.0f, 6.0f);
        }
    }

    if (IsDead())
    {
        walk_phase_ = 0;

        roll_ += 50.0 * delta_time;
        if (roll_ > 90.0)
            roll_ = 90.0;

        if (!dropped_disk_)
        {
            ItemID disk_id = static_cast<ItemID>(RNG{}.Range(static_cast<uint8_t>(ItemID::disk), static_cast<uint8_t>(ItemID::disk18)));
            DroppedItem *disk = new DroppedItem({
                .position = position_,
                .item = disk_id,
                .amount = 1
            });
            disk->SetVelocity({
                RNG{}.Range(-2.0f, 2.0f),
                RNG{}.Range(2.0f, 4.0f),
                RNG{}.Range(-2.0f, 2.0f)
            });
            Moon::GetCurrentMoon()->GetEntityManager().AddEntity(disk);

            dropped_disk_ = true;
        }

        if (time_since_death_ > 4.0f)
            death_animation_done_ = true;
    }
    else if (action_ == AstronautAction::ROTATE_LEFT)
    {
        yaw_ -= 100.0f * delta_time;
        walk_phase_ += 0.5f * delta_time;
    }
    else if (action_ == AstronautAction::ROTATE_RIGHT)
    {
        yaw_ += 100.0f * delta_time;
        walk_phase_ += 0.5f * delta_time;
    }
    else if (action_ == AstronautAction::MOVE)
    {
        auto displacement_this_frame = position_ - position_last_frame_;
        displacement_this_frame.y = 0;
        float distance_this_frame = glm::length(displacement_this_frame);
        bool blocked = distance_this_frame < 0.5f * glm::length(move_velocity_) * delta_time;
        if (blocked && walk_phase_ > 0.1f)
        {
            action_ = RNG{}.Range(0, 1) == 0 ? AstronautAction::ROTATE_LEFT : AstronautAction::ROTATE_RIGHT;
            move_velocity_ = -0.3f * move_velocity_;
        }

        walk_phase_ += 2.0f * delta_time;
    }
    else if (action_ == AstronautAction::ATTACK)
    {
        walk_phase_ += 2.0f * delta_time;

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
            action_ = AstronautAction::NONE;
            next_action_time_ += 1.0f;
        }
    }
    else if (action_ == AstronautAction::NONE)
    {
        move_velocity_ = glm::vec3{0};
        walk_phase_ *= 0.999f;
    }

    velocity_.x = move_velocity_.x;
    velocity_.z = move_velocity_.z;
    position_last_frame_ = position_;
}

void Astronaut::FixedUpdate(){}

void Astronaut::Render(const glm::mat4 &view, const glm::mat4 &proj)
{
    auto RotateAround = [](glm::vec3 pivot, glm::quat rot)
    {
        return glm::translate(glm::mat4(1.0f), pivot) *
            glm::mat4_cast(rot) *
            glm::translate(glm::mat4(1.0f), -pivot);
    };

    glm::vec4 fog_color = Moon::GetCurrentMoon()->GetFogColor();
    float render_distance = OptionsManager::GetOptions().render_distance;

    glm::vec4 color;
    if (level_ == 0)
        color = {1.0, 1.0, 1.0, 1.0}; // white
    else if (level_ == 1)
        color = {0.85, 1.0, 0.85, 1.0}; // mint green
    else if (level_ == 2)
        color = {0.85, 0.85, 1.0, 1.0}; // powder blue
    else if (level_ == 3)
        color = {1.0, 0.85, 0.85, 1.0}; // pink
    else
        color = {1.0, 1.0, 0.6, 1.0}; // yellow

    if (friendly_)
    {
        color.r *= 0.5f;
        color.g = 1.0f - pain_time_;
        color.b *= 0.5f * (1.0f - pain_time_);
    }
    else
    {
        color.g *= 1.0f - pain_time_;
        color.b *= 1.0f - pain_time_;
    }

    auto &shader = ShaderManager::MOB_SHADER;
    shader.Use();
    shader.SetMat4("u_view", view);
    shader.SetMat4("u_proj", proj);
    shader.SetVec4("u_fog_color", fog_color);
    shader.SetFloat("u_fog_start", (float)render_distance * 0.33f * 32.0f);
    shader.SetFloat("u_fog_end", (float)render_distance * 0.85f * 32.0f);
    shader.SetVec4("u_color", color);

    float walk_spread = glm::radians(12.0f * glm::sin(walk_phase_));

    glm::mat4 root_transform =
        glm::translate(glm::mat4(1.0f), position_) *
        glm::mat4_cast(glm::angleAxis(glm::radians(yaw_), glm::vec3(0, 1, 0))) *
        glm::scale(glm::mat4{1.0f}, glm::vec3{0.82f});

    // Right leg
    glm::mat4 leg_right = root_transform;
    leg_right = glm::translate(leg_right, glm::vec3{-0.35f, -1.25f, 0.0f});
    leg_right *= RotateAround(
        glm::vec3{0.25f, -1.8f, -0.5f} + glm::vec3{0.35f, 1.45f, 0},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    leg_right *= RotateAround(
        {0.0f, 0.75f, 0.0f},
        glm::angleAxis(walk_spread, glm::vec3(-1, 0, 0))
    );
    leg_right = glm::scale(leg_right, glm::vec3{0.325f, 0.75f, 0.325f});
    limb_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", leg_right);
    });

    // Left leg
    glm::mat4 leg_left = root_transform;
    leg_left = glm::translate(leg_left, glm::vec3{0.35f, -1.25f, 0.0f});
    leg_left *= RotateAround(
        glm::vec3{0.25f, -1.8f, -0.5f} + glm::vec3{-0.35f, 1.45f, 0},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    leg_left *= RotateAround(
        {0.0f, 0.75f, 0.0f},
        glm::angleAxis(walk_spread, glm::vec3(1, 0, 0))
    );
    leg_left = glm::scale(leg_left, glm::vec3{0.325f, 0.75f, 0.325f});
    limb_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", leg_left);
    });

    // Left arm
    glm::mat4 arm_left = root_transform;
    arm_left = glm::translate(arm_left, glm::vec3{0.75f, 0.4f, 0.0f});
    arm_left *= RotateAround(
        glm::vec3{0.25f, -1.8f, -0.5f} + glm::vec3{-0.75f, -0.2f, 0},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    arm_left = glm::scale(arm_left, glm::vec3{0.175f, 0.625f, 0.175f});
    limb_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", arm_left);
    });

    // Right arm
    glm::mat4 arm_right = root_transform;
    arm_right = glm::translate(arm_right, glm::vec3{-0.75f, 0.9f, 0.625f});
    arm_right *= RotateAround(
        glm::vec3{0.25f, -1.8f, -0.5f} + glm::vec3{0.8f, -0.7f, 0},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    arm_right *= glm::mat4_cast(glm::angleAxis(glm::radians(90.0f), glm::vec3(-1, 0, 0)));
    arm_right = glm::scale(arm_right, glm::vec3{0.175f, 0.625f, 0.175f});
    limb_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", arm_right);
    });

    // Pistol
    glm::mat4 pistol = root_transform;
    pistol = glm::translate(pistol, glm::vec3{-0.75f, 1.2f, 1.3f});
    pistol *= RotateAround(
        glm::vec3{0.25f, -1.8f, -0.5f} + glm::vec3{0.8f, -1.0f, 0},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    pistol = glm::scale(pistol, glm::vec3{0.2f, 0.2f, 0.45f});
    pistol_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", pistol);
    });

    // Body
    glm::mat4 body = root_transform;
    body = glm::translate(body, glm::vec3{0.0f, 0.2f, 0.0f});
    body *= RotateAround(
        glm::vec3{0.25f, -1.8f, -0.5f},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    body = glm::scale(body, glm::vec3{0.6f, 0.75f, 0.6f});
    body_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", body);
    });

    // Head
    glm::mat4 head = root_transform;
    head = glm::translate(head, glm::vec3{0.0f, 1.45f, 0.0f});
    head *= RotateAround(
        glm::vec3{0.25f, -1.8f, -0.5f} + glm::vec3{0, -1.25f, 0},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    head = glm::scale(head, glm::vec3{0.5f, 0.5f, 0.5f});
    head_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", head);
    });

    // Backpack
    glm::mat4 backpack = root_transform;
    backpack = glm::translate(backpack, glm::vec3{0.0f, 0.7f, -0.975f});
    backpack *= RotateAround(
        glm::vec3{0.25f, -1.8f, -0.5f} + glm::vec3{0, -0.5f, 0},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    backpack = glm::scale(backpack, glm::vec3{0.6f, 0.65f, 0.375f});
    backpack_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", backpack);
    });
}

AstronautData Astronaut::GetAstronautData()
{
    return {
        .id = id_,
        .position = position_,
        .velocity = velocity_,
        .move_velocity = move_velocity_,
        .target_entity_id = target_entity_id_,
        .yaw = yaw_,
        .health = health_,
        .level = level_,
        .action = static_cast<uint8_t>(action_),
        .friendly = friendly_
    };
}

void Astronaut::NotifyOfAttacker(size_t id)
{
    if (id == Moon::GetCurrentMoon()->GetPlayer()->GetID())
        friendly_ = false;

    if (health_ > 0 && id != GetID())
    {
        target_entity_id_ = id;
        action_ = AstronautAction::ATTACK;
        next_action_time_ = internal_time_;
    }
}
