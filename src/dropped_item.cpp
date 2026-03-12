
#include "dropped_item.h"
#include "block.h"
#include "helpers.h"
#include "storage.h"
#include "moon.h"
#include "sound_system.h"

DroppedItem::DroppedItem(DroppedItemData data)
{
    type_ = EntityType::DROPPED_ITEM;
    position_ = data.position;
    prev_position_ = data.position;
    next_position_ = data.position;
    aabb_.center = data.position;
    aabb_.extents = {0.125f, 0.125f, 0.125f};
    item_ = data.item;
    amount_ = data.amount;
    lifetime_ = data.lifetime;
    death_animation_done_ = true;

    mesh_.SetShader(ShaderManager::SIMPLE_UNLIT_SHADER);

    if (ItemIsSprite(item_) || item_ == ItemID::minilight)
    {
        float vertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        };

        auto item_icon = GetItemIcon(item_);
        mesh_.SetTexture(item_icon.bytes, item_icon.width, item_icon.height, item_icon.num_channels, GL_NEAREST);
        mesh_.SetVertexData(vertices, sizeof(vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);
    }
    else
    {
        auto tile_origins = GetAtlasTileOrigins();
        glm::vec2 tile_origin_bottom = tile_origins[ItemIDToBlockID(item_)][2];
        glm::vec2 tile_origin_side = tile_origins[ItemIDToBlockID(item_)][1];
        glm::vec2 tile_origin_top = tile_origins[ItemIDToBlockID(item_)][0];
        float tile_size = 1.0f / 14.0f;
        float vertices[] = {
            // Front
            -1.0f, -1.0f, 1.0f, tile_origin_side.x,             tile_origin_side.y,
             1.0f, -1.0f, 1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
             1.0f,  1.0f, 1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,
             1.0f,  1.0f, 1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,
            -1.0f,  1.0f, 1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
            -1.0f, -1.0f, 1.0f, tile_origin_side.x,             tile_origin_side.y,

            // Back
             1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,
             1.0f, -1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
            -1.0f, -1.0f, -1.0f, tile_origin_side.x,             tile_origin_side.y,
            -1.0f, -1.0f, -1.0f, tile_origin_side.x,             tile_origin_side.y,
            -1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
             1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,

            // Left
            -1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,
            -1.0f, -1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
            -1.0f, -1.0f,  1.0f, tile_origin_side.x,             tile_origin_side.y,
            -1.0f, -1.0f,  1.0f, tile_origin_side.x,             tile_origin_side.y,
            -1.0f,  1.0f,  1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
            -1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,

            // Right
            1.0f, -1.0f,  1.0f, tile_origin_side.x,             tile_origin_side.y,
            1.0f, -1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
            1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,
            1.0f,  1.0f, -1.0f, tile_origin_side.x + tile_size, tile_origin_side.y + tile_size,
            1.0f,  1.0f,  1.0f, tile_origin_side.x + tile_size, tile_origin_side.y,
            1.0f, -1.0f,  1.0f, tile_origin_side.x,             tile_origin_side.y,

            // Top
             1.0f, 1.0f,  1.0f, tile_origin_top.x + tile_size, tile_origin_top.y + tile_size,
             1.0f, 1.0f, -1.0f, tile_origin_top.x + tile_size, tile_origin_top.y,
            -1.0f, 1.0f, -1.0f, tile_origin_top.x,             tile_origin_top.y,
            -1.0f, 1.0f, -1.0f, tile_origin_top.x,             tile_origin_top.y,
            -1.0f, 1.0f,  1.0f, tile_origin_top.x + tile_size, tile_origin_top.y,
             1.0f, 1.0f,  1.0f, tile_origin_top.x + tile_size, tile_origin_top.y + tile_size,

            // Bottom
            -1.0f, -1.0f, -1.0f, tile_origin_bottom.x,             tile_origin_bottom.y,
             1.0f, -1.0f, -1.0f, tile_origin_bottom.x + tile_size, tile_origin_bottom.y,
             1.0f, -1.0f,  1.0f, tile_origin_bottom.x + tile_size, tile_origin_bottom.y + tile_size,
             1.0f, -1.0f,  1.0f, tile_origin_bottom.x + tile_size, tile_origin_bottom.y + tile_size,
            -1.0f, -1.0f,  1.0f, tile_origin_bottom.x + tile_size, tile_origin_bottom.y,
            -1.0f, -1.0f, -1.0f, tile_origin_bottom.x,             tile_origin_bottom.y,
        };

        mesh_.SetTexture(Storage::IMAGES / "texture_atlas.png");
        mesh_.SetVertexData(vertices, sizeof(vertices) / (5 * sizeof(float)), GL_STATIC_DRAW);
    }
}

void DroppedItem::Update(float delta_time)
{
    lifetime_ += delta_time;

    if (lifetime_ > 120.0f) // Dropped items last for 2 minutes
        SetIsDead(true);

    if (is_grounded_ && !moving_toward_player_)
    {
        bob_time_ += delta_time;
        rotate_time_ += delta_time;
    }
    else
    {
        bob_time_ = 0;
    }
}

void DroppedItem::FixedUpdate()
{
    auto player_pos = Moon::GetCurrentMoon()->GetPlayer()->GetPosition();
    auto &player_inventory = Moon::GetCurrentMoon()->GetPlayer()->GetInventory();

    float distance = glm::distance(player_pos, position_);

    moving_toward_player_ = distance < 2.5f && player_inventory.HasSpaceForItem(item_);

    if (moving_toward_player_)
        velocity_ = 4.0f * glm::normalize(player_pos - position_);
    else if (is_grounded_)
        velocity_ = glm::vec3{0, velocity_.y, 0};

    if (distance < 0.5f)
    {
        SetIsDead(true);
        player_inventory.Add({item_, amount_});
        SoundSystem::Play(SoundSystem::Sound::PICKUP);
    }
}

void DroppedItem::Render(const glm::mat4 &vp_matrix)
{
    // Sprites rotate to face the player, while blocks just rotate around.
    // Both bob up and down when not moving towards player.

    // These will just have an AABB around them, but they won't collide with the world
    // along X or Z axes.

    float bob_offset = 0.08f * glm::sin(2.0f * bob_time_);

    glm::mat4 model{1.0f};
    model = glm::translate(model, {position_.x, position_.y + 0.1f + bob_offset, position_.z});
    if (ItemIsSprite(item_))
    {
        auto player_pos = Moon::GetCurrentMoon()->GetPlayer()->GetPosition();
        auto displacement = player_pos - position_;
        displacement.y = 0;
        auto forward = glm::normalize(displacement);
        auto up = glm::vec3{0, 1, 0};
        auto right = glm::normalize(glm::cross(up, forward));

        glm::mat4 rotation = glm::mat4(
            glm::vec4(right,   0.0f),
            glm::vec4(up,      0.0f),
            glm::vec4(forward, 0.0f),
            glm::vec4(0.0f, 0.0f , 0.0f, 1.0f)
        );

        model = model * rotation;
    }
    else
    {
        float yaw = (2.0f * 3.1416f / 10.0f) * rotate_time_;
        model = glm::rotate(model, yaw, {0, 1, 0});
    }

    model = glm::scale(model, glm::vec3{ItemIsSprite(item_) ? 0.2f : 0.15f});

    mesh_.Render([&](Shader *shader) {
        shader->SetMat4("u_mvp_matrix", vp_matrix * model);
    });
}

DroppedItemData DroppedItem::GetDroppedItemData()
{
    return {
        .position = position_,
        .item = item_,
        .amount = amount_,
        .lifetime = lifetime_
    };
}

ItemID DroppedItem::GetItem()
{
    return item_;
}

int DroppedItem::GetAmount()
{
    return amount_;
}
