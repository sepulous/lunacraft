
#include "entity.h"
#include "mesh.h"

struct GreenMobData
{
    glm::vec3 position;
    float yaw;
    int health;
};

class GreenMob : public Entity
{
    public:
        GreenMob(GreenMobData data);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &view, const glm::mat4 &proj) override;

        GreenMobData GetGreenMobData();

    private:
        enum class GreenMobAction {ROTATE, JUMP, NONE};

    private:
        Mesh mesh_;
        float next_action_time_ = 0;
        GreenMobAction action_ = GreenMobAction::NONE;
        glm::vec3 jump_vector_;
        float time_since_death_ = 0;
        bool dropped_biogel_ = false;
        float target_yaw_;
        float yaw_;
        float roll_;
};
