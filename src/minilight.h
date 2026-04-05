#pragma once

#include "entity.h"
#include "mesh.h"

struct MinilightData
{
    glm::ivec3 voxel;
    glm::vec3 normal;
};

class Minilight : public Entity
{
    public:
        Minilight(glm::ivec3 voxel, glm::vec3 normal);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &view, const glm::mat4 &proj) override;

        glm::ivec3 GetVoxel();
        MinilightData GetMinilightData();

    private:
        Mesh mesh_;
        glm::ivec3 voxel_;
        glm::vec3 normal_;
};
