
#include "entity.h"
#include "mesh.h"

struct BrownMobData
{
    glm::vec3 position;
    float yaw = 0;
    int health = 280;
};

class BrownMob : public Entity
{
    public:
        BrownMob(BrownMobData data);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &view, const glm::mat4 &proj) override;

        BrownMobData GetBrownMobData();
        void NotifyOfAttacker(size_t id);

    private:
        enum class BrownMobAction {NONE, MOVE, ROTATE_LEFT, ROTATE_RIGHT, CHASE};

    private:
        Mesh mesh_;
        glm::vec3 move_velocity_;
        BrownMobAction action_ = BrownMobAction::NONE;
        size_t target_entity_id_ = 0;
        float internal_time_ = 0;
        float next_action_time_ = 0;
        float time_since_death_ = 0;
        float yaw_;
};
