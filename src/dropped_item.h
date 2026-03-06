#pragma once

#include "entity.h"
#include "item.h"
#include "mesh.h"

class DroppedItem : public Entity
{
    public:
        DroppedItem(ItemID item, int amount, glm::vec3 position);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &vp_matrix) override;

        ItemID GetItem();
        int GetAmount();

    private:
        Mesh mesh_;
        ItemID item_;
        int amount_;
        float bob_time_ = 0;
        float rotate_time_ = 0;
        bool moving_toward_player_ = false;
};
