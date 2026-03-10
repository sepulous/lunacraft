#pragma once

#include <vector>

#include "chunk_manager.h"
#include "entity.h"

class EntityManager
{
    private:
        std::vector<Entity *> entities_;
        std::vector<Entity *> entities_to_spawn_;

    public:
        EntityManager() = default;
        ~EntityManager();

        EntityManager(const EntityManager&) = delete;
        EntityManager &operator=(const EntityManager&) = delete;

        EntityManager(EntityManager&&) = delete;
        EntityManager &operator=(EntityManager&&) = delete;

        void AddEntity(Entity *entity);
        void FixedUpdate();
        void Update(float delta_time);
        void PhysicsStep();
        void Interpolate(double interp);
        void RenderEntities(const glm::mat4 &vp_matrix);
        void DestroyMinilightAt(glm::ivec3 voxel);
        void LoadInitialEntities();
        void LoadChunkEntities(glm::ivec3 chunk_coords);
        void UnloadChunkEntities(glm::ivec3 chunk_coords);
        void SaveAllEntities();
};
