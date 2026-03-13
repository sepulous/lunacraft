#pragma once

#include <vector>
#include <unordered_map>

#include "chunk_manager.h"
#include "entity.h"

class EntityManager
{
    private:
        std::unordered_map<size_t, Entity *> entities_;
        std::vector<Entity *> entities_to_spawn_;
        size_t next_entity_id_ = 0;

    public:
        EntityManager() = default;
        ~EntityManager();

        EntityManager(const EntityManager&) = delete;
        EntityManager &operator=(const EntityManager&) = delete;

        EntityManager(EntityManager&&) = delete;
        EntityManager &operator=(EntityManager&&) = delete;

        void AddEntity(Entity *entity);
        Entity *GetEntityByID(size_t id);
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
