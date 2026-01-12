
#include <iostream>

#include "entity_manager.h"
#include "helpers.h"

EntityManager::~EntityManager()
{
    for (Entity *entity : _entities)
        delete entity;
}

void EntityManager::LinkChunkManager(ChunkManager *chunk_manager)
{
    _chunk_manager = chunk_manager;
}

void EntityManager::AddEntity(Entity *entity)
{
    _entities.push_back(entity);
}

void EntityManager::FixedUpdate()
{
    for (Entity *entity : _entities)
        entity->FixedUpdate();
}

void EntityManager::Update()
{
    for (Entity *entity : _entities)
        entity->Update();
}

void EntityManager::RunPhysics(int steps, float interp)
{
    auto TestAABBWorld = [this](const AABB& box) {
        float min_x = glm::round(box.center.x - box.extents.x);
        float max_x = glm::round(box.center.x + box.extents.x);
        float min_y = glm::round(box.center.y - box.extents.y);
        float max_y = glm::round(box.center.y + box.extents.y);
        float min_z = glm::round(box.center.z - box.extents.z);
        float max_z = glm::round(box.center.z + box.extents.z);

        auto &chunks = _chunk_manager->GetChunks();
        for (int x = min_x; x <= max_x; x++)
        {
            for (int y = min_y; y <= max_y; y++)
            {
                for (int z = min_z; z <= max_z; z++)
                {
                    glm::ivec3 box_chunk_coords = VoxelToChunk({x, y, z});
                    uint64_t chunk_id = ChunkCoordsToID(box_chunk_coords);

                    auto it = chunks.find(chunk_id);
                    if (it != chunks.end())
                    {
                        glm::ivec3 local_block_pos = GlobalToLocalVoxel({x, y, z});
                        if ((BlockID)it->second.GetBlocks()[GetChunkIndex(local_block_pos.x, local_block_pos.y, local_block_pos.z)] != BlockID::air)
                            return true;
                    }
                }
            }
        }

        return false;
    };

    for (Entity *entity : _entities)
    {
        glm::vec3 next_position = entity->GetPosition();
        for (int i = 0; i < steps; i++)
        {
            entity->SetPrevPosition(entity->GetPosition());

            // Gravity
            entity->SetVelocity(entity->GetVelocity() - glm::vec3(0, 4.0f * FIXED_DELTA_TIME, 0));

            // X
            next_position.x += entity->GetVelocity().x * FIXED_DELTA_TIME;
            entity->GetAABB().center.x = next_position.x;
            if (TestAABBWorld(entity->GetAABB()))
            {
                next_position.x = entity->GetPosition().x; // Don't actually move
                entity->GetAABB().center.x = entity->GetPosition().x;
                entity->SetVelocity({0, entity->GetVelocity().y, entity->GetVelocity().z});
            }

            // Z
            next_position.z += entity->GetVelocity().z * FIXED_DELTA_TIME;
            entity->GetAABB().center.z = next_position.z;
            if (TestAABBWorld(entity->GetAABB()))
            {
                next_position.z = entity->GetPosition().z; // Don't actually move
                entity->GetAABB().center.z = entity->GetPosition().z;
                entity->SetVelocity({entity->GetVelocity().x, entity->GetVelocity().y, 0});
            }

            // Y
            next_position.y += entity->GetVelocity().y * FIXED_DELTA_TIME;
            entity->GetAABB().center.y = next_position.y;
            if (TestAABBWorld(entity->GetAABB()))
            {
                if (entity->GetVelocity().y <= 0)
                    entity->SetGrounded(true);
                next_position.y = entity->GetPosition().y; // Don't actually move
                entity->GetAABB().center.y = entity->GetPosition().y;
                entity->SetVelocity({entity->GetVelocity().x, 0, entity->GetVelocity().z});
            }
            else
            {
                entity->SetGrounded(false);
            }

            entity->SetNextPosition(next_position);
        }

        // Interpolate position
        entity->SetPosition(glm::mix(entity->GetPrevPosition(), entity->GetNextPosition(), interp));
        entity->GetAABB().center = entity->GetPosition();
    }
}
