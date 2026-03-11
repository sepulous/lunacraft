
#include "entity.h"
#include "mesh.h"

struct GreenMobData
{
    glm::vec3 position;
    float rotation; // yaw
    int health;
};

class GreenMob : public Entity
{
    public:
        GreenMob(GreenMobData data);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &vp_matrix) override;

        GreenMobData GetGreenMobData();

    private:
        enum class GreenMobAction {ROTATE, JUMP, NONE};

    private:
        Mesh mesh_;
        float rotation_;
        int health_;
        float damage_time_ = 0;
        float next_action_time_ = 0;
        GreenMobAction action_ = GreenMobAction::NONE;
        glm::vec3 jump_vector_;
        float rotation_angle_;
};
