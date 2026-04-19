
#include "giraffe.h"
#include "moon.h"
#include "options.h"
#include "storage.h"
#include "rng.h"

Giraffe::Giraffe(GiraffeData data)
{
    type_ = EntityType::GIRAFFE;
    position_ = data.position;
    prev_position_ = data.position;
    next_position_ = data.position;
    yaw_ = data.yaw;
    roll_ = 0;
    health_ = data.health;
    aabb_.center = data.position;
    aabb_.extents = {0.5f, 1.0f, 0.5f};

    //
    // Leg mesh
    //

    glm::vec2 leg_size = {2.0f / 16.0f, 16.0f / 50.0f};
    glm::vec2 leg_bl = {8.0f / 16.0f, 14.0f / 50.0f};
    glm::vec2 leg_tr = leg_bl + leg_size;

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
         1.0f, 0.0f,  1.0f, leg_tr.x, leg_bl.y + (2.0f / 50.0f),
         1.0f, 0.0f, -1.0f, leg_tr.x, leg_bl.y,
        -1.0f, 0.0f, -1.0f, leg_bl.x, leg_bl.y,
        -1.0f, 0.0f, -1.0f, leg_bl.x, leg_bl.y,
        -1.0f, 0.0f,  1.0f, leg_bl.x, leg_bl.y + (2.0f / 50.0f),
         1.0f, 0.0f,  1.0f, leg_tr.x, leg_bl.y + (2.0f / 50.0f),

        // Bottom
        -1.0f, -2.0f, -1.0f, leg_bl.x, leg_bl.y,
         1.0f, -2.0f, -1.0f, leg_tr.x, leg_bl.y,
         1.0f, -2.0f,  1.0f, leg_tr.x, leg_bl.y + (2.0f / 50.0f),
         1.0f, -2.0f,  1.0f, leg_tr.x, leg_bl.y + (2.0f / 50.0f),
        -1.0f, -2.0f,  1.0f, leg_bl.x, leg_bl.y + (2.0f / 50.0f),
        -1.0f, -2.0f, -1.0f, leg_bl.x, leg_bl.y,
    };
    leg_mesh_.SetShader(ShaderManager::MOB_SHADER);
    leg_mesh_.SetTexture(Storage::IMAGES / "entities" / "giraffe.png");
    leg_mesh_.SetVertexData(leg_vertices, sizeof(leg_vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);

    //
    // Neck mesh
    //

    glm::vec2 neck_size = {6.0f / 16.0f, 1.0f};
    glm::vec2 neck_bl = {10.0f / 16.0f, 0.0f};
    glm::vec2 neck_tr = neck_bl + neck_size;

    float neck_vertices[] = {
        // Front
        -1.0f, -1.0f, 1.0f, neck_bl.x, neck_bl.y,
         1.0f, -1.0f, 1.0f, neck_tr.x, neck_bl.y,
         1.0f,  1.0f, 1.0f, neck_tr.x, neck_tr.y,
         1.0f,  1.0f, 1.0f, neck_tr.x, neck_tr.y,
        -1.0f,  1.0f, 1.0f, neck_bl.x, neck_tr.y,
        -1.0f, -1.0f, 1.0f, neck_bl.x, neck_bl.y,

        // Back
         1.0f,  1.0f, -1.0f, neck_tr.x, neck_tr.y - (3.0f / 50.0f),
         1.0f, -1.0f, -1.0f, neck_tr.x, neck_bl.y,
        -1.0f, -1.0f, -1.0f, neck_bl.x, neck_bl.y,
        -1.0f, -1.0f, -1.0f, neck_bl.x, neck_bl.y,
        -1.0f,  1.0f, -1.0f, neck_bl.x, neck_tr.y - (3.0f / 50.0f),
         1.0f,  1.0f, -1.0f, neck_tr.x, neck_tr.y - (3.0f / 50.0f),

        // Left
        -1.0f,  1.0f, -1.0f, neck_tr.x, neck_tr.y - (3.0f / 50.0f),
        -1.0f, -1.0f, -1.0f, neck_tr.x, neck_bl.y,
        -1.0f, -1.0f,  1.0f, neck_bl.x, neck_bl.y,
        -1.0f, -1.0f,  1.0f, neck_bl.x, neck_bl.y,
        -1.0f,  1.0f,  1.0f, neck_bl.x, neck_tr.y - (3.0f / 50.0f),
        -1.0f,  1.0f, -1.0f, neck_tr.x, neck_tr.y - (3.0f / 50.0f),

        // Right
        1.0f, -1.0f,  1.0f, neck_bl.x, neck_bl.y,
        1.0f, -1.0f, -1.0f, neck_tr.x, neck_bl.y,
        1.0f,  1.0f, -1.0f, neck_tr.x, neck_tr.y - (3.0f / 50.0f),
        1.0f,  1.0f, -1.0f, neck_tr.x, neck_tr.y - (3.0f / 50.0f),
        1.0f,  1.0f,  1.0f, neck_bl.x, neck_tr.y - (3.0f / 50.0f),
        1.0f, -1.0f,  1.0f, neck_bl.x, neck_bl.y,

        // Top
         1.0f, 1.0f,  1.0f, neck_tr.x, neck_tr.y - (3.0f / 50.0f),
         1.0f, 1.0f, -1.0f, neck_tr.x, neck_bl.y,
        -1.0f, 1.0f, -1.0f, neck_bl.x, neck_bl.y,
        -1.0f, 1.0f, -1.0f, neck_bl.x, neck_bl.y,
        -1.0f, 1.0f,  1.0f, neck_bl.x, neck_tr.y - (3.0f / 50.0f),
         1.0f, 1.0f,  1.0f, neck_tr.x, neck_tr.y - (3.0f / 50.0f),

        // Bottom
        -1.0f, -1.0f, -1.0f, neck_bl.x, neck_bl.y,
         1.0f, -1.0f, -1.0f, neck_tr.x, neck_bl.y,
         1.0f, -1.0f,  1.0f, neck_tr.x, neck_tr.y - (3.0f / 50.0f),
         1.0f, -1.0f,  1.0f, neck_tr.x, neck_tr.y - (3.0f / 50.0f),
        -1.0f, -1.0f,  1.0f, neck_bl.x, neck_tr.y - (3.0f / 50.0f),
        -1.0f, -1.0f, -1.0f, neck_bl.x, neck_bl.y,
    };
    neck_mesh_.SetShader(ShaderManager::MOB_SHADER);
    neck_mesh_.SetTexture(Storage::IMAGES / "entities" / "giraffe.png");
    neck_mesh_.SetVertexData(neck_vertices, sizeof(neck_vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);

    //
    // Body mesh
    //

    glm::vec2 body_side_size = {10.0f / 16.0f, 6.0f / 50.0f};
    glm::vec2 body_side_bl = {0.0f, 8.0f / 50.0f};
    glm::vec2 body_side_tr = body_side_bl + body_side_size;

    glm::vec2 body_fb_size = {8.0f / 16.0f, 6.0f / 50.0f};
    glm::vec2 body_fb_bl = {0.0f, 14.0f / 50.0f};
    glm::vec2 body_fb_tr = body_fb_bl + body_fb_size;

    glm::vec2 body_tb_size = {10.0f / 16.0f, 8.0f / 50.0f};
    glm::vec2 body_tb_bl = {0.0f, 0.0f};
    glm::vec2 body_tb_tr = body_tb_bl + body_tb_size;

    float body_vertices[] = {
        // Front
        -1.0f, -1.0f, 1.0f, body_fb_bl.x, body_fb_bl.y,
         1.0f, -1.0f, 1.0f, body_fb_tr.x, body_fb_bl.y,
         1.0f,  1.0f, 1.0f, body_fb_tr.x, body_fb_tr.y,
         1.0f,  1.0f, 1.0f, body_fb_tr.x, body_fb_tr.y,
        -1.0f,  1.0f, 1.0f, body_fb_bl.x, body_fb_tr.y,
        -1.0f, -1.0f, 1.0f, body_fb_bl.x, body_fb_bl.y,

        // Back
         1.0f,  1.0f, -1.0f, body_fb_tr.x, body_fb_tr.y,
         1.0f, -1.0f, -1.0f, body_fb_tr.x, body_fb_bl.y,
        -1.0f, -1.0f, -1.0f, body_fb_bl.x, body_fb_bl.y,
        -1.0f, -1.0f, -1.0f, body_fb_bl.x, body_fb_bl.y,
        -1.0f,  1.0f, -1.0f, body_fb_bl.x, body_fb_tr.y,
         1.0f,  1.0f, -1.0f, body_fb_tr.x, body_fb_tr.y,

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
         1.0f, 1.0f,  1.0f, body_tb_tr.x, body_tb_tr.y,
         1.0f, 1.0f, -1.0f, body_tb_tr.x, body_tb_bl.y,
        -1.0f, 1.0f, -1.0f, body_tb_bl.x, body_tb_bl.y,
        -1.0f, 1.0f, -1.0f, body_tb_bl.x, body_tb_bl.y,
        -1.0f, 1.0f,  1.0f, body_tb_bl.x, body_tb_tr.y,
         1.0f, 1.0f,  1.0f, body_tb_tr.x, body_tb_tr.y,

        // Bottom
        -1.0f, -1.0f, -1.0f, body_tb_bl.x, body_tb_bl.y,
         1.0f, -1.0f, -1.0f, body_tb_tr.x, body_tb_bl.y,
         1.0f, -1.0f,  1.0f, body_tb_tr.x, body_tb_tr.y,
         1.0f, -1.0f,  1.0f, body_tb_tr.x, body_tb_tr.y,
        -1.0f, -1.0f,  1.0f, body_tb_bl.x, body_tb_tr.y,
        -1.0f, -1.0f, -1.0f, body_tb_bl.x, body_tb_bl.y,
    };
    body_mesh_.SetShader(ShaderManager::MOB_SHADER);
    body_mesh_.SetTexture(Storage::IMAGES / "entities" / "giraffe.png");
    body_mesh_.SetVertexData(body_vertices, sizeof(body_vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);
}

void Giraffe::Update(float delta_time)
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
        time_since_death_ += delta_time;
    }
    else
    {
        if (internal_time_ > next_action_time_)
        {
            if (action_ == GiraffeAction::NONE)
            {
                float chance = RNG{}.Range(0.0f, 1.0f);
                if (chance < 0.3f) // Walk
                {
                    action_ = GiraffeAction::WALK;
                    next_action_time_ += RNG{}.Range(5.0f, 10.0f);

                    walk_velocity_ = glm::vec3{glm::sin(yaw_), 0, glm::cos(yaw_)};
                }
                else if (chance < 0.5f)
                {
                    action_ = GiraffeAction::ROTATE_LEFT;
                    next_action_time_ += RNG{}.Range(0.0f, 3.0f);
                }
                else if (chance < 0.7f)
                {
                    action_ = GiraffeAction::ROTATE_RIGHT;
                    next_action_time_ += RNG{}.Range(0.0f, 3.0f);
                }
                else if (chance < 0.9f)
                {
                    action_ = GiraffeAction::WALK;
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
                action_ = GiraffeAction::NONE;
                next_action_time_ += RNG{}.Range(2.0f, 13.0f);
                time_walking_ = 0;
                velocity_.x = 0;
                velocity_.z = 0;
            }
        }
    }
}

void Giraffe::FixedUpdate()
{
    if (IsDead())
    {
        velocity_.x = 0;
        velocity_.z = 0;

        if (!death_animation_done_)
        {
            roll_ = glm::clamp(90.0f * time_since_death_, 0.0f, 90.0f); // 90 degrees/sec
            if (time_since_death_ > 4.0f)
                death_animation_done_ = true;
        }
    }
    else if (action_ == GiraffeAction::ROTATE_LEFT)
    {
        yaw_ -= glm::radians(25.0f) * FIXED_DELTA_TIME;
    }
    else if (action_ == GiraffeAction::ROTATE_RIGHT)
    {
        yaw_ += glm::radians(25.0f) * FIXED_DELTA_TIME;
    }
    else if (action_ == GiraffeAction::WALK)
    {
        time_walking_ += FIXED_DELTA_TIME;
        velocity_.x = walk_velocity_.x;
        velocity_.z = walk_velocity_.z;
    }
}

void Giraffe::Render(const glm::mat4 &view, const glm::mat4 &proj)
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

    if (action_ == GiraffeAction::WALK || glm::abs(neck_bob_) > 0.02f || glm::abs(walk_spread_) > 0.02f)
    {
        if (action_ == GiraffeAction::WALK)
        {
            neck_bob_ = glm::radians(2.0f * glm::sin(2.0f * time_walking_));
            walk_spread_ = glm::radians(12.0f * glm::sin(3.0f * time_walking_));
        }
        else
        {
            neck_bob_ *= 0.99f;
            walk_spread_ *= 0.99f;
        }
    }
    else
    {
        neck_bob_ = 0;
        walk_spread_ = 0;
    }

    glm::mat4 root_transform =
        glm::translate(glm::mat4(1.0f), position_) *
        glm::mat4_cast(glm::angleAxis(yaw_, glm::vec3(0, 1, 0)));

    // Front right leg
    glm::mat4 leg_fr = root_transform;
    leg_fr = glm::translate(leg_fr, glm::vec3{-0.25f, 1.1f, 0.5f});
    leg_fr *= RotateAround(
        {0.25f, -1.8f, -0.5f},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    leg_fr *= glm::mat4_cast(glm::angleAxis(walk_spread_, glm::vec3(-1, 0, 0)));
    leg_fr = glm::scale(leg_fr, glm::vec3{0.125f, 1.1f, 0.125f});
    leg_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", leg_fr);
    });

    // Front left leg
    glm::mat4 leg_fl = root_transform;
    leg_fl = glm::translate(leg_fl, glm::vec3{0.25f, 1.1f, 0.5f});
    leg_fl *= RotateAround(
        {-0.25f, -1.6f, -0.5f},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    leg_fl *= glm::mat4_cast(glm::angleAxis(walk_spread_, glm::vec3(1, 0, 0)));
    leg_fl = glm::scale(leg_fl, glm::vec3{0.125f, 1.1f, 0.125f});
    leg_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", leg_fl);
    });

    // Back right leg
    glm::mat4 leg_br = root_transform;
    leg_br = glm::translate(leg_br, glm::vec3{-0.25f, 1.0f, -0.5f});
    leg_br *= RotateAround(
        {0.25f, -1.8f, 0.5f},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    leg_br *= glm::mat4_cast(glm::angleAxis(walk_spread_, glm::vec3(1, 0, 0)));
    leg_br = glm::scale(leg_br, glm::vec3{0.125f, 1.0f, 0.125f});
    leg_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", leg_br);
    });

    // Back left leg
    glm::mat4 leg_bl = root_transform;
    leg_bl = glm::translate(leg_bl, glm::vec3{0.25f, 1.0f, -0.5f});
    leg_bl *= RotateAround(
        {-0.25f, -1.6f, 0.5f},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    leg_bl *= glm::mat4_cast(glm::angleAxis(walk_spread_, glm::vec3(-1, 0, 0)));
    leg_bl = glm::scale(leg_bl, glm::vec3{0.125f, 1.0f, 0.125f});
    leg_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", leg_bl);
    });

    // Body
    glm::mat4 body = root_transform;
    body = glm::translate(body, {0, 1.3f, 0});
    body *= RotateAround(
        {0, -1.8f, 0},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    body *= glm::mat4_cast(glm::angleAxis(glm::radians(15.0f), glm::vec3(-1, 0, 0)));
    body = glm::scale(body, {0.5f, 0.375f, 0.575f});
    body_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", body);
    });

    // Neck
    glm::mat4 neck = root_transform;
    neck = glm::translate(neck, {0, 2.1f, 0.75f});
    neck *= RotateAround(
        {0, -2.6f, 0},
        glm::angleAxis(glm::radians(roll_), glm::vec3(0, 0, 1)) // local forward
    );
    neck *= glm::mat4_cast(glm::angleAxis(glm::radians(15.0f) + neck_bob_, glm::vec3(1, 0, 0)));
    neck = glm::scale(neck, {0.125f, 1.0f, 0.125f});
    neck_mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_model", neck);
    });
}

GiraffeData Giraffe::GetGiraffeData()
{
    return {
        .position = position_,
        .yaw = yaw_,
        .health = health_
    };
}
