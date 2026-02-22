#pragma once

#include <vector>

#include "chunk_manager.h"
#include "entity.h"

class EntityManager
{
    private:
        ChunkManager *_chunk_manager;
        std::vector<Entity *> _entities;

    public:
        EntityManager() = default;
        ~EntityManager();
        void LinkChunkManager(ChunkManager *chunk_manager);
        void AddEntity(Entity *entity);
        void FixedUpdate();
        void Update(float delta_time);
        void RunPhysics(int steps, float interp);
};
