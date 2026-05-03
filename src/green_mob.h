
#include "entity.h"
#include "mesh.h"

struct GreenMobData
{
    size_t id = 0;
    glm::vec3 position;
    float yaw = 0;
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
        enum class GreenMobAction {NONE, MOVE, ROTATE_LEFT, ROTATE_RIGHT};

    private:
        Mesh mesh_;
        glm::vec3 move_velocity_;
        GreenMobAction action_ = GreenMobAction::NONE;
        float internal_time_ = 0;
        float next_action_time_ = 0;
        float time_since_death_ = 0;
        float yaw_;
        float roll_;
        bool dropped_biogel_ = false;
};
