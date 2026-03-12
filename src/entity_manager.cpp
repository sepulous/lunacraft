
#include <iostream>

#include <fstream>
#include <filesystem>

#include "entity_manager.h"
#include "helpers.h"
#include "minilight.h"
#include "storage.h"
#include "moon.h"
#include "slug.h"
#include "green_mob.h"
#include "dropped_item.h"

EntityManager::~EntityManager()
{
    for (Entity *entity : entities_)
        delete entity;
}

void EntityManager::LoadInitialEntities()
{
    int moon_id = Moon::GetCurrentMoon()->GetID();
    std::filesystem::path entity_folder = Storage::MOONS / (std::string("moon") + std::to_string(moon_id)) / "entities";
    if (std::filesystem::exists(entity_folder))
    {
        auto chunks = Moon::GetCurrentMoon()->GetChunkManager().GetAllChunks();
        for (auto chunk : chunks)
        {
            if (chunk->IsBorderChunk())
                continue;

            auto chunk_id = ChunkCoordsToID(chunk->GetCoords());
            std::filesystem::path entity_file_path = entity_folder / (std::to_string(chunk_id) + ".dat");
            if (std::filesystem::exists(entity_file_path))
            {
                std::ifstream entity_file(entity_file_path, std::ios::binary);

                EntityType type;
                while (entity_file.read(reinterpret_cast<char *>(&type), sizeof(EntityType)))
                {
                    if (type == EntityType::MINILIGHT)
                    {
                        MinilightData data;
                        entity_file.read(reinterpret_cast<char *>(&data), sizeof(MinilightData));
                        AddEntity(new Minilight(data.voxel, data.normal));
                    }
                    else if (type == EntityType::DROPPED_ITEM)
                    {
                        DroppedItemData data;
                        entity_file.read(reinterpret_cast<char *>(&data), sizeof(DroppedItemData));
                        AddEntity(new DroppedItem(data));
                    }
                    else if (type == EntityType::SLUG)
                    {
                        SlugData data;
                        entity_file.read(reinterpret_cast<char *>(&data), sizeof(SlugData));
                        AddEntity(new Slug(data));
                    }
                    else if (type == EntityType::GREEN_MOB)
                    {
                        GreenMobData data;
                        entity_file.read(reinterpret_cast<char *>(&data), sizeof(GreenMobData));
                        AddEntity(new GreenMob(data));
                    }
                }

                entity_file.close();
            }
        }
    }
}

void EntityManager::AddEntity(Entity *entity)
{
    entities_to_spawn_.push_back(entity);
}

void EntityManager::FixedUpdate()
{
    for (Entity *entity : entities_to_spawn_)
        entities_.push_back(entity);
    entities_to_spawn_.clear();

    for (Entity *entity : entities_)
        entity->FixedUpdate();
}

void EntityManager::Update(float delta_time)
{
    std::erase_if(entities_, [](Entity *e) { return e->IsDead() && e->IsDeathAnimationDone(); });

    for (Entity *entity : entities_)
        entity->Update(delta_time);
}

void EntityManager::RenderEntities(const glm::mat4 &vp_matrix)
{
    for (Entity *entity : entities_)
        entity->Render(vp_matrix);
}

void EntityManager::PhysicsStep()
{
    auto &chunk_manager = Moon::GetCurrentMoon()->GetChunkManager();

    auto TestAABBWorld = [this, &chunk_manager](const AABB &box) {
        float min_x = glm::round(box.center.x - box.extents.x);
        float max_x = glm::round(box.center.x + box.extents.x);
        float min_y = glm::round(box.center.y - box.extents.y);
        float max_y = glm::round(box.center.y + box.extents.y);
        float min_z = glm::round(box.center.z - box.extents.z);
        float max_z = glm::round(box.center.z + box.extents.z);

        for (int x = min_x; x <= max_x; x++)
        {
            for (int z = min_z; z <= max_z; z++)
            {
                glm::ivec3 box_chunk_coords = VoxelToChunk({x, 0, z});
                auto chunk = chunk_manager.GetChunk(box_chunk_coords);
                if (chunk != nullptr)
                {
                    for (int y = min_y; y <= max_y; y++)
                    {
                        glm::ivec3 local_block_pos = GlobalToLocalVoxel({x, y, z});
                        BlockID block = chunk->GetBlocks()[GetChunkIndex(local_block_pos)];
                        if (block != BlockID::air && block != BlockID::minilight)
                            return true;
                    }
                }
            }
        }

        return false;
    };

    auto TestSlugWorld = [&chunk_manager](const glm::vec3 &position) {
        auto voxel = GetNearestVoxel(position);
        auto chunk_coords = VoxelToChunk(voxel);
        auto chunk = chunk_manager.GetChunk(chunk_coords);
        if (chunk != nullptr)
        {
            auto block = chunk->GetBlocks()[GetChunkIndex(GlobalToLocalVoxel(voxel))];
            return block != BlockID::air && block != BlockID::minilight;
        }
        else
        {
            return true;
        }
    };

    for (Entity *entity : entities_)
    {
        if (entity->GetType() == EntityType::MINILIGHT)
            continue;

        if (entity->GetType() == EntityType::SLUG)
        {
            Slug *slug = dynamic_cast<Slug *>(entity);
            if (slug->IsFlying())
            {
                Entity *hit_entity = nullptr;
                for (Entity *other : entities_)
                {
                    if (other->CanBeDamaged())
                    {
                        auto slug_pos = slug->GetPosition();
                        auto &other_aabb = other->GetAABB();
                        auto other_aabb_min = other_aabb.center - other_aabb.extents;
                        auto other_aabb_max = other_aabb.center + other_aabb.extents;
                        if (glm::distance(glm::clamp(slug_pos, other_aabb_min, other_aabb_max), slug_pos) < 0.01f)
                        {
                            hit_entity = other;
                            break;
                        }
                    }
                }

                if (hit_entity != nullptr)
                {
                    int new_health = hit_entity->GetHealth() - slug->GetSlugData().damage;
                    if (new_health <= 0)
                    {
                        hit_entity->SetHealth(0);
                        hit_entity->SetIsDead(true);
                    }
                    else
                    {
                        hit_entity->SetHealth(new_health);
                    }

                    slug->SetIsDead(true);
                }
                else
                {
                    slug->SetPrevPosition(slug->GetPosition());

                    glm::vec3 next_position = slug->GetPosition() + glm::vec3{FIXED_DELTA_TIME * slug->GetVelocity()};
                    slug->SetNextPosition(next_position);

                    if (TestSlugWorld(next_position))
                    {
                        slug->SetIsFlying(false);
                        slug->SetPrevPosition(next_position);
                        slug->SetPosition(next_position);
                    }
                }
            }
        }
        else
        {
            glm::vec3 next_position = entity->GetPosition();
            entity->SetPrevPosition(entity->GetPosition());

            // Gravity
            if (!entity->IsGrounded())
                entity->SetVelocity(entity->GetVelocity() - glm::vec3(0, GRAVITY * FIXED_DELTA_TIME, 0));

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
                {
                    entity->SetGrounded(true);

                    // Snap feet to avoid sinking
                    float floor_feet = glm::floor(entity->GetPosition().y - entity->GetAABB().extents.y);
                    next_position.y = floor_feet + entity->GetAABB().extents.y + 0.5f;
                    entity->GetAABB().center.y = next_position.y;
                }
                else
                {
                    next_position.y = entity->GetPosition().y;
                    entity->GetAABB().center.y = entity->GetPosition().y;
                }
                
                entity->SetVelocity({entity->GetVelocity().x, 0, entity->GetVelocity().z});
            }
            else
            {
                // Decide whether on ice
                auto entity_voxel_g = GetNearestVoxel(entity->GetPosition());
                auto entity_voxel_l = GlobalToLocalVoxel(entity_voxel_g);
                auto entity_chunk = chunk_manager.GetChunk(VoxelToChunk(entity_voxel_g));
                BlockID foot_block = entity_chunk->GetBlocks()[GetChunkIndex(entity_voxel_l - glm::ivec3{0, 1, 0})]; // Can go out of bounds...
                entity->SetIsOnIce(foot_block == BlockID::water);

                // Decide whether still grounded
                float actual_feet = entity->GetPosition().y - entity->GetAABB().extents.y;
                float floor_feet = glm::floor(actual_feet);
                if (glm::abs(actual_feet - (floor_feet + 0.5f)) > 0.01f || foot_block == BlockID::air)
                    entity->SetGrounded(false);
            }

            entity->SetNextPosition(next_position);
        }
    }
}

void EntityManager::Interpolate(double interp)
{
    for (Entity *entity : entities_)
    {
        entity->SetPosition(glm::mix(entity->GetPrevPosition(), entity->GetNextPosition(), interp));
        entity->GetAABB().center = entity->GetPosition();
    }
}

void EntityManager::DestroyMinilightAt(glm::ivec3 voxel)
{
    for (auto it = entities_.begin(); it != entities_.end(); ++it)
    {
        Entity *entity = *it;
        if (entity->GetType() == EntityType::MINILIGHT)
        {
            Minilight *minilight = dynamic_cast<Minilight *>(entity);
            if (minilight->GetVoxel() == voxel)
            {
                entities_.erase(it);
                break;
            }
        }
    }
}

void EntityManager::LoadChunkEntities(glm::ivec3 chunk_coords)
{
    int moon_id = Moon::GetCurrentMoon()->GetID();
    std::filesystem::path entity_folder = Storage::MOONS / (std::string("moon") + std::to_string(moon_id)) / "entities";
    if (std::filesystem::exists(entity_folder))
    {
        auto chunk_id = ChunkCoordsToID(chunk_coords);
        std::filesystem::path entity_file_path = entity_folder / (std::to_string(chunk_id) + ".dat");
        if (std::filesystem::exists(entity_file_path))
        {
            std::ifstream entity_file(entity_file_path, std::ios::binary);

            EntityType type;
            while (entity_file.read(reinterpret_cast<char *>(&type), sizeof(EntityType)))
            {
                if (type == EntityType::MINILIGHT)
                {
                    MinilightData data;
                    entity_file.read(reinterpret_cast<char *>(&data), sizeof(MinilightData));
                    AddEntity(new Minilight(data.voxel, data.normal));
                }
                else if (type == EntityType::DROPPED_ITEM)
                {
                    DroppedItemData data;
                    entity_file.read(reinterpret_cast<char *>(&data), sizeof(DroppedItemData));
                    AddEntity(new DroppedItem(data));
                }
                else if (type == EntityType::SLUG)
                {
                    SlugData data;
                    entity_file.read(reinterpret_cast<char *>(&data), sizeof(SlugData));
                    AddEntity(new Slug(data));
                }
                else if (type == EntityType::GREEN_MOB)
                {
                    GreenMobData data;
                    entity_file.read(reinterpret_cast<char *>(&data), sizeof(GreenMobData));
                    AddEntity(new GreenMob(data));
                }
            }

            entity_file.close();
        }
    }
}

void EntityManager::UnloadChunkEntities(glm::ivec3 chunk_coords)
{
    int moon_id = Moon::GetCurrentMoon()->GetID();
    auto chunk_id = ChunkCoordsToID(chunk_coords);

    std::filesystem::path entity_folder = Storage::MOONS / (std::string("moon") + std::to_string(moon_id)) / "entities";
    if (!std::filesystem::exists(entity_folder))
        std::filesystem::create_directory(entity_folder);

    bool chunk_has_entities = false;
    for (auto entity : entities_)
    {
        auto entity_chunk_coords = VoxelToChunk(GetNearestVoxel(entity->GetPosition()));
        if (entity->GetType() != EntityType::PLAYER && entity_chunk_coords == chunk_coords)
        {
            chunk_has_entities = true;
            break;
        }
    }

    if (chunk_has_entities)
    {
        std::filesystem::path entity_file_path = entity_folder / (std::to_string(chunk_id) + ".dat");
        std::ofstream entity_file(entity_file_path, std::ios::binary);
        
        for (auto it = entities_.begin(); it != entities_.end(); )
        {
            auto entity = *it;
            EntityType type = entity->GetType();
            if (type == EntityType::PLAYER)
            {
                ++it;
                continue;
            }

            auto entity_chunk_coords = VoxelToChunk(GetNearestVoxel(entity->GetPosition()));
            if (entity_chunk_coords == chunk_coords)
            {
                entity_file.write(reinterpret_cast<const char *>(&type), sizeof(EntityType));

                if (type == EntityType::MINILIGHT)
                {
                    MinilightData data = dynamic_cast<Minilight *>(entity)->GetMinilightData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(MinilightData));
                }
                else if (type == EntityType::DROPPED_ITEM)
                {
                    DroppedItemData data = dynamic_cast<DroppedItem *>(entity)->GetDroppedItemData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(DroppedItemData));
                }
                else if (type == EntityType::SLUG)
                {
                    SlugData data = dynamic_cast<Slug *>(entity)->GetSlugData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(SlugData));
                }
                else if (type == EntityType::GREEN_MOB)
                {
                    GreenMobData data = dynamic_cast<GreenMob *>(entity)->GetGreenMobData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(GreenMobData));
                }

                delete entity;
                it = entities_.erase(it);
            }
            else
            {
                ++it;
            }    
        }

        entity_file.close();
    }
}

void EntityManager::SaveAllEntities()
{
    int moon_id = Moon::GetCurrentMoon()->GetID();
    std::filesystem::path entity_folder = Storage::MOONS / (std::string("moon") + std::to_string(moon_id)) / "entities";
    if (!std::filesystem::exists(entity_folder))
        std::filesystem::create_directory(entity_folder);

    std::unordered_map<uint64_t, std::vector<Entity *>> entity_chunk_map;
    for (Entity *entity : entities_)
    {
        if (entity->GetType() == EntityType::PLAYER)
            continue;

        auto entity_pos = entity->GetPosition();
        auto entity_chunk_coords = VoxelToChunk(GetNearestVoxel(entity_pos));
        auto chunk_id = ChunkCoordsToID(entity_chunk_coords);
        
        if (entity_chunk_map.contains(chunk_id))
            entity_chunk_map.at(chunk_id).push_back(entity);
        else
            entity_chunk_map.insert({chunk_id, {entity}});
    }

    auto chunks = Moon::GetCurrentMoon()->GetChunkManager().GetAllChunks();
    for (auto chunk : chunks)
    {
        auto chunk_id = chunk->GetID();
        std::filesystem::path entity_file_path = entity_folder / (std::to_string(chunk_id) + ".dat");
        if (entity_chunk_map.contains(chunk_id))
        {
            std::ofstream entity_file(entity_file_path, std::ios::binary);
            auto &chunk_entities = entity_chunk_map.at(chunk_id);

            for (Entity *entity : chunk_entities)
            {
                EntityType type = entity->GetType();
                entity_file.write(reinterpret_cast<const char *>(&type), sizeof(EntityType));

                if (type == EntityType::MINILIGHT)
                {
                    MinilightData data = dynamic_cast<Minilight *>(entity)->GetMinilightData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(MinilightData));
                }
                else if (type == EntityType::DROPPED_ITEM)
                {
                    DroppedItemData data = dynamic_cast<DroppedItem *>(entity)->GetDroppedItemData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(DroppedItemData));
                }
                else if (type == EntityType::SLUG)
                {
                    SlugData data = dynamic_cast<Slug *>(entity)->GetSlugData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(SlugData));
                }
                else if (type == EntityType::GREEN_MOB)
                {
                    GreenMobData data = dynamic_cast<GreenMob *>(entity)->GetGreenMobData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(GreenMobData));
                }
            }

            entity_file.close();
        }
        else if (std::filesystem::exists(entity_file_path)) // Chunk has no entities
        {
            std::filesystem::remove(entity_file_path);
        }
    }
}
