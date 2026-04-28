
#include <cstdint>

#include "entity.h"
#include "mesh.h"

struct AstronautData
{
    size_t id = 0;
    glm::vec3 position;
    glm::vec3 velocity{0};
    glm::vec3 move_velocity{0};
    size_t target_entity_id = 0;
    float yaw = 0;
    int health = 100;
    int level = 0;
    uint8_t action = 0;
    bool friendly = false;
};

class Astronaut : public Entity
{
    public:
        Astronaut(AstronautData data);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &view, const glm::mat4 &proj) override;

        AstronautData GetAstronautData();
        void NotifyOfAttacker(size_t id);

    private:
        enum class AstronautAction : uint8_t {NONE, MOVE, ROTATE_LEFT, ROTATE_RIGHT, ATTACK};

    private:
        Mesh body_mesh_;
        Mesh limb_mesh_;
        Mesh head_mesh_;
        Mesh pistol_mesh_;
        Mesh backpack_mesh_;
        glm::vec3 move_velocity_;
        glm::vec3 position_last_frame_;
        AstronautAction action_ = AstronautAction::NONE;
        int level_;
        size_t target_entity_id_;
        float walk_phase_ = 0;
        float internal_time_ = 0;
        float next_action_time_ = 0;
        float time_since_death_ = 0;
        float yaw_ = 0;
        float roll_ = 0;
        bool friendly_;
        bool dropped_disk_ = false;
};
