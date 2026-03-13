
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
        void Render(const glm::mat4 &vp_matrix) override;

        BrownMobData GetBrownMobData();
        void NotifyOfAttacker(size_t id);

    private:
        enum class BrownMobAction {ROTATE, JUMP, NONE};

    private:
        Mesh mesh_;
        float next_action_time_ = 0;
        BrownMobAction action_ = BrownMobAction::NONE;
        glm::vec3 jump_vector_;
        size_t attacker_id_;
        float time_since_death_ = 0;
        float target_yaw_;
        float yaw_;
        bool aggressive_ = false;
};
