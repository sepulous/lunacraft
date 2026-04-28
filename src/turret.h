
#include <cstdint>

#include "entity.h"
#include "mesh.h"

struct TurretData
{
    size_t id = 0;
    glm::vec3 position;
    float yaw;
    int health = 100;
    int level = 0; // 0, 2, 4
    size_t target_entity_id;
    uint8_t action;
};

class Turret : public Entity
{
    public:
        Turret(TurretData data);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &view, const glm::mat4 &proj) override;

        TurretData GetTurretData();
        void NotifyOfAttacker(size_t id);

    private:
        enum class TurretAction : uint8_t {NONE, ROTATE_LEFT, ROTATE_RIGHT, ATTACK};

    private:
        Mesh mesh_;
        TurretAction action_ = TurretAction::NONE;
        int level_;
        size_t target_entity_id_;
        float internal_time_ = 0;
        float next_action_time_ = 0;
        float time_since_death_ = 0;
        float yaw_;
};
