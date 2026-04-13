
#include "entity.h"
#include "mesh.h"
#include "inventory.h"

struct BlueMobData
{
    glm::vec3 position;
    ItemStack stolen_item = {ItemID::none, 0};
    float yaw = 0;
    int health = 30;
    bool chasing_player = false;
};

class BlueMob : public Entity
{
    public:
        BlueMob(BlueMobData data);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &view, const glm::mat4 &proj) override;

        BlueMobData GetBlueMobData();
        void NotifyOfAttacker(size_t id);

    private:
        enum class BlueMobAction {WALK, JUMP_AND_WALK, ROTATE, NONE};

    private:
        Mesh body_mesh_;
        Mesh leg_mesh_;
        BlueMobAction action_ = BlueMobAction::NONE;
        glm::vec3 jump_vector_;
        ItemStack stolen_item_;
        float next_action_time_ = 0;
        float time_since_death_ = 0;
        float time_walking_ = 0;
        float walk_spread_ = 0;
        float target_yaw_;
        float yaw_;
        float roll_;
        bool chasing_player_ = false;
};
