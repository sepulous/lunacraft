#pragma once

#include <vector>

#include "chunk_manager.h"
#include "entity.h"

class EntityManager
{
    private:
        ChunkManager *chunk_manager_;
        std::vector<Entity *> entities_;

    public:
        EntityManager() = default;
        ~EntityManager();
        void LinkChunkManager(ChunkManager *chunk_manager);
        void AddEntity(Entity *entity);
        void FixedUpdate();
        void Update(float delta_time);
        void PhysicsStep();
        void Interpolate(double interp);
};
