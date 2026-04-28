#pragma once

#include "entity.h"
#include "item.h"
#include "mesh.h"

struct DroppedItemData
{
    size_t id = 0;
    glm::vec3 position;
    ItemID item;
    int amount;
    float lifetime = 0;
};

class DroppedItem : public Entity
{
    public:
        DroppedItem(DroppedItemData data);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &view, const glm::mat4 &proj) override;

        DroppedItemData GetDroppedItemData();

        ItemID GetItem();
        int GetAmount();

    private:
        Mesh mesh_;
        ItemID item_;
        int amount_;
        float lifetime_ = 0;
        float bob_time_ = 0;
        float rotate_time_ = 0;
        bool moving_toward_player_ = false;
};
