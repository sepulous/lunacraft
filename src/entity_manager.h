#pragma once

#include <vector>
#include <optional>
#include <unordered_map>

#include "chunk_manager.h"
#include "entity.h"

class EntityManager
{
    private:
        std::unordered_map<size_t, Entity *> entities_;
        std::vector<Entity *> entities_to_spawn_;
        size_t entity_count_ = 0;

    public:
        EntityManager() = default;
        ~EntityManager();

        EntityManager(const EntityManager&) = delete;
        EntityManager &operator=(const EntityManager&) = delete;

        EntityManager(EntityManager&&) = delete;
        EntityManager &operator=(EntityManager&&) = delete;

        void AddEntity(Entity *entity);
        Entity *GetEntityByID(size_t id);
        Entity *GetNearestMob(size_t ref_entity_id, float max_distance = 1000.0f);
        void SelfUpdate();
        void Update(float delta_time);
        void RunPhysics(double &accumulator);
        void RenderEntities(const glm::mat4 &view, const glm::mat4 &proj);
        void DestroyMinilightAt(glm::ivec3 voxel);
        bool DestroyItemNear(ItemID item_id, glm::vec3 position, float max_distance);
        void LoadInitialEntities();
        void LoadChunkEntities(glm::ivec3 chunk_coords);
        void UnloadChunkEntities(glm::ivec3 chunk_coords);
        void SaveAllEntities();
        void SetEntityCount(size_t count);
        size_t GetEntityCount();

    private:
        void Integrate(Entity *entity);
};
