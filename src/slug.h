
#include "entity.h"
#include "mesh.h"

struct SlugData
{
    glm::vec3 initial_position;
    glm::vec3 initial_velocity;
    float time_flying = 0;
    float time_stuck = 0;
    bool is_flying = true;
};

class Slug : public Entity
{
    public:
        Slug(SlugData slug_data);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &vp_matrix) override;

        SlugData GetSlugData();

        void SetIsFlying(bool value);
        bool IsFlying();

    private:
        Mesh mesh_;
        glm::vec3 initial_position_;
        glm::vec3 initial_velocity_;
        float time_flying_;
        float time_stuck_;
        bool is_flying_;
};
