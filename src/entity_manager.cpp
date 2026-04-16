
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
#include "brown_mob.h"
#include "blue_mob.h"
#include "giraffe.h"
#include "dropped_item.h"

EntityManager::~EntityManager()
{
    for (auto [entity_id, entity] : entities_)
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
                    else if (type == EntityType::BROWN_MOB)
                    {
                        BrownMobData data;
                        entity_file.read(reinterpret_cast<char *>(&data), sizeof(BrownMobData));
                        AddEntity(new BrownMob(data));
                    }
                    else if (type == EntityType::GIRAFFE)
                    {
                        GiraffeData data;
                        entity_file.read(reinterpret_cast<char *>(&data), sizeof(GiraffeData));
                        AddEntity(new Giraffe(data));
                    }
                    else if (type == EntityType::BLUE_MOB)
                    {
                        BlueMobData data;
                        entity_file.read(reinterpret_cast<char *>(&data), sizeof(BlueMobData));
                        AddEntity(new BlueMob(data));
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

Entity *EntityManager::GetEntityByID(size_t id)
{
    if (!entities_.contains(id))
        return nullptr;

    return entities_.at(id);
}

std::optional<size_t> EntityManager::GetNearestEntityID(size_t ref_entity_id, EntityType type, float max_distance)
{
    auto ref_entity = GetEntityByID(ref_entity_id);
    if (!ref_entity)
        return std::nullopt;

    bool success = false;
    size_t nearest_entity_id;
    float nearest_distance = 10000.0f;

    auto ref_entity_pos = ref_entity->GetPosition();
    for (auto [entity_id, entity] : entities_)
    {
        auto entity_pos = entity->GetPosition();
        float distance = glm::length(entity_pos - ref_entity_pos);
        if (entity_id != ref_entity_id && distance < nearest_distance && distance < max_distance && (type == EntityType::ANY || entity->GetType() == type))
        {
            nearest_entity_id = entity_id;
            nearest_distance = distance;
            success = true;
        }
    }

    return success ? std::optional<size_t>{nearest_entity_id} : std::nullopt;
}

void EntityManager::SelfUpdate()
{
    // Global lighting
    float sin_world_time = glm::sin(Moon::GetCurrentMoon()->GetSkyboxAngleClamped() + glm::radians(5.0f));
    float entity_lighting = glm::clamp(sin_world_time, 0.8f, 1.0f);
    auto &mob_shader = ShaderManager::MOB_SHADER;
    mob_shader.Use();
    mob_shader.SetFloat("u_lighting", entity_lighting);

    // Remove old entities
    std::erase_if(entities_, [](const auto &pair) {
        const auto &[_, entity] = pair;
        return entity->IsDead() && entity->IsDeathAnimationDone();
    });

    // Add new entities
    for (Entity *entity : entities_to_spawn_)
    {
        entity->SetID(next_entity_id_);
        entities_.emplace(next_entity_id_, entity);
        next_entity_id_++;
    }
    entities_to_spawn_.clear();
}

void EntityManager::Update(float delta_time)
{
    for (auto [entity_id, entity] : entities_)
        entity->Update(delta_time);
}

void EntityManager::RenderEntities(const glm::mat4 &view, const glm::mat4 &proj)
{
    for (auto [entity_id, entity] : entities_)
        entity->Render(view, proj);
}

void EntityManager::Integrate(Entity *entity)
{
    auto &chunk_manager = Moon::GetCurrentMoon()->GetChunkManager();

    auto TestAABBWorld = [this, &chunk_manager](const AABB &box) {
        float min_x = glm::round(box.center.x - box.extents.x);
        float max_x = glm::round(box.center.x + box.extents.x);
        float min_y = glm::round(box.center.y - box.extents.y);
        float max_y = glm::round(box.center.y + box.extents.y);
        float min_z = glm::round(box.center.z - box.extents.z);
        float max_z = glm::round(box.center.z + box.extents.z);

        if (max_y >= (float)WORLD_HEIGHT_LIMIT)
            return false;

        for (int x = min_x; x <= max_x; x++)
        {
            for (int z = min_z; z <= max_z; z++)
            {
                glm::ivec3 box_chunk_coords = VoxelToChunk({x, 0, z});
                auto chunk = chunk_manager.GetChunk(box_chunk_coords);
                if (chunk)
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
        
        if (voxel.y >= WORLD_HEIGHT_LIMIT) return false;

        auto chunk_coords = VoxelToChunk(voxel);
        auto chunk = chunk_manager.GetChunk(chunk_coords);
        if (chunk)
        {
            auto block = chunk->GetBlocks()[GetChunkIndex(GlobalToLocalVoxel(voxel))];
            return block != BlockID::air && block != BlockID::minilight;
        }
        
        return false;
    };

    if (entity->GetType() == EntityType::MINILIGHT)
        return;

    if (entity->GetType() == EntityType::SLUG)
    {
        Slug *slug = dynamic_cast<Slug *>(entity);
        SlugData slug_data = slug->GetSlugData();
        if (slug->IsFlying())
        {
            Entity *hit_entity = nullptr;
            for (auto [_, other] : entities_)
            {
                if (other->CanBeDamaged() && slug_data.source_id != other->GetID())
                {
                    auto slug_offset = 0.2f * glm::normalize(slug->GetVelocity());
                    auto slug_pos = slug->GetPosition() + slug_offset;
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
                hit_entity->Damage(slug_data.damage);
                slug->SetIsDead(true);

                auto entity_type = hit_entity->GetType();
                if (entity_type == EntityType::BROWN_MOB)
                    dynamic_cast<BrownMob *>(hit_entity)->NotifyOfAttacker(slug_data.source_id);
                else if (entity_type == EntityType::BLUE_MOB)
                    dynamic_cast<BlueMob *>(hit_entity)->NotifyOfAttacker(slug_data.source_id);
            }
            else
            {
                slug->SetPrevPosition(slug->GetPosition());

                glm::vec3 next_position = slug->GetPosition() + glm::vec3{FIXED_DELTA_TIME * slug->GetVelocity()};
                slug->SetNextPosition(next_position);

                glm::vec3 offset = 0.2f * glm::normalize(slug->GetVelocity());
                if (TestSlugWorld(next_position + offset))
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
        glm::vec3 current_velocity = entity->GetVelocity();
        auto &aabb = entity->GetAABB();

        glm::vec3 current_next_position = entity->GetNextPosition();
        entity->SetPrevPosition(current_next_position);
        glm::vec3 new_next_position = current_next_position;

        // Gravity
        if (!entity->IsGrounded())
        {
            float gravity = entity->GetType() == EntityType::PLAYER ? PLAYER_GRAVITY : MOB_GRAVITY;
            entity->SetVelocity(current_velocity - glm::vec3(0, gravity * FIXED_DELTA_TIME, 0));
        }

        // X
        new_next_position.x += current_velocity.x * FIXED_DELTA_TIME;
        aabb.center.x = new_next_position.x;
        if (TestAABBWorld(aabb))
        {
            new_next_position.x = current_next_position.x; // Don't actually move
            aabb.center.x = current_next_position.x;
            entity->SetVelocity({0, current_velocity.y, current_velocity.z});
        }

        // Z
        new_next_position.z += current_velocity.z * FIXED_DELTA_TIME;
        aabb.center.z = new_next_position.z;
        if (TestAABBWorld(aabb))
        {
            new_next_position.z = current_next_position.z; // Don't actually move
            aabb.center.z = current_next_position.z;
            entity->SetVelocity({current_velocity.x, current_velocity.y, 0});
        }

        // Y
        new_next_position.y += current_velocity.y * FIXED_DELTA_TIME;
        aabb.center.y = new_next_position.y;
        if (TestAABBWorld(aabb))
        {
            if (current_velocity.y <= 0)
            {
                entity->SetGrounded(true);

                // Snap feet to avoid sinking
                float floor_feet = glm::floor(current_next_position.y - aabb.extents.y);
                new_next_position.y = floor_feet + aabb.extents.y + 0.5f;
                aabb.center.y = new_next_position.y;
            }
            else
            {
                new_next_position.y = current_next_position.y;
                aabb.center.y = current_next_position.y;
            }
            
            entity->SetVelocity({current_velocity.x, 0, current_velocity.z});
        }
        else
        {
            auto entity_voxel_g = GetNearestVoxel(current_next_position);
            if (entity_voxel_g.y > 0 && entity_voxel_g.y < WORLD_HEIGHT_LIMIT)
            {
                auto entity_voxel_l = GlobalToLocalVoxel(entity_voxel_g);
                auto entity_chunk = chunk_manager.GetChunk(VoxelToChunk(entity_voxel_g));
                BlockID foot_block = entity_chunk->GetBlocks()[GetChunkIndex(entity_voxel_l - glm::ivec3{0, 1, 0})];

                // Decide whether on ice
                entity->SetIsOnIce(foot_block == BlockID::water);

                // Decide whether still grounded
                float actual_feet = current_next_position.y - aabb.extents.y;
                float floor_feet = glm::floor(actual_feet);
                if (glm::abs(actual_feet - (floor_feet + 0.5f)) > 0.01f || foot_block == BlockID::air)
                    entity->SetGrounded(false);
            }
        }

        entity->SetNextPosition(new_next_position);
    }
}

void EntityManager::RunPhysics(double &accumulator)
{
    while (accumulator >= FIXED_DELTA_TIME)
    {
        for (auto [_, entity] : entities_)
        {
            entity->FixedUpdate();
            Integrate(entity);
        }
        accumulator -= FIXED_DELTA_TIME;
    }

    float alpha = accumulator / FIXED_DELTA_TIME;
    for (auto [_, entity] : entities_)
    {
        entity->SetPosition(glm::mix(
            entity->GetPrevPosition(),
            entity->GetNextPosition(),
            alpha
        ));
    }
}

void EntityManager::DestroyMinilightAt(glm::ivec3 voxel)
{
    for (auto it = entities_.begin(); it != entities_.end(); ++it)
    {
        Entity *entity = it->second;
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
                else if (type == EntityType::BROWN_MOB)
                {
                    BrownMobData data;
                    entity_file.read(reinterpret_cast<char *>(&data), sizeof(BrownMobData));
                    AddEntity(new BrownMob(data));
                }
                else if (type == EntityType::GIRAFFE)
                {
                    GiraffeData data;
                    entity_file.read(reinterpret_cast<char *>(&data), sizeof(GiraffeData));
                    AddEntity(new Giraffe(data));
                }
                else if (type == EntityType::BLUE_MOB)
                {
                    BlueMobData data;
                    entity_file.read(reinterpret_cast<char *>(&data), sizeof(BlueMobData));
                    AddEntity(new BlueMob(data));
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
    for (auto [entity_id, entity] : entities_)
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
            auto entity = it->second;
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
                else if (type == EntityType::BROWN_MOB)
                {
                    BrownMobData data = dynamic_cast<BrownMob *>(entity)->GetBrownMobData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(BrownMobData));
                }
                else if (type == EntityType::GIRAFFE)
                {
                    GiraffeData data = dynamic_cast<Giraffe *>(entity)->GetGiraffeData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(GiraffeData));
                }
                else if (type == EntityType::BLUE_MOB)
                {
                    BlueMobData data = dynamic_cast<BlueMob *>(entity)->GetBlueMobData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(BlueMobData));
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
    for (auto [entity_id, entity] : entities_)
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
                else if (type == EntityType::BROWN_MOB)
                {
                    BrownMobData data = dynamic_cast<BrownMob *>(entity)->GetBrownMobData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(BrownMobData));
                }
                else if (type == EntityType::GIRAFFE)
                {
                    GiraffeData data = dynamic_cast<Giraffe *>(entity)->GetGiraffeData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(GiraffeData));
                }
                else if (type == EntityType::BLUE_MOB)
                {
                    BlueMobData data = dynamic_cast<BlueMob *>(entity)->GetBlueMobData();
                    entity_file.write(reinterpret_cast<const char *>(&data), sizeof(BlueMobData));
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
