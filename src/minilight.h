#pragma once

#include "entity.h"
#include "mesh.h"

enum class MinilightDir
{
    POSITIVE_X,
    NEGATIVE_X,
    POSITIVE_Y,
    NEGATIVE_Y,
    POSITIVE_Z,
    NEGATIVE_Z
};

struct MinilightData
{
    glm::ivec3 voxel;
    MinilightDir dir;
};

class Minilight : public Entity
{
    public:
        Minilight(glm::ivec3 voxel, MinilightDir dir);
        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &vp_matrix) override;

        glm::ivec3 GetVoxel();
        MinilightData GetMinilightData();

    private:
        Mesh mesh_;
        glm::ivec3 voxel_;
        MinilightDir dir_;
};
