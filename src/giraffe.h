
#include "entity.h"
#include "mesh.h"

struct GiraffeData
{
    glm::vec3 position;
    float yaw = 0;
    int health = 100;
};

class Giraffe : public Entity
{
    public:
        Giraffe(GiraffeData data);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &view, const glm::mat4 &proj) override;

        GiraffeData GetGiraffeData();

    private:
        enum class GiraffeAction {WALK, ROTATE_LEFT, ROTATE_RIGHT, NONE};

    private:
        Mesh body_mesh_;
        Mesh leg_mesh_;
        Mesh neck_mesh_;
        GiraffeAction action_ = GiraffeAction::NONE;
        glm::vec3 walk_velocity_;
        float internal_time_ = 0;
        float next_action_time_ = 0;
        float time_since_death_ = 0;
        float time_walking_ = 0;
        float walk_spread_ = 0;
        float neck_bob_ = 0;
        float yaw_;
        float roll_;
};
