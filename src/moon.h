#pragma once

#include <cstdint>

#include <glm/glm.hpp>

#include "moon_settings.h"
#include "entity_manager.h"
#include "skybox.h"
#include "player.h"
#include "mesh.h"

class ChunkManager;

class SelectionBlock
{
    public:
        SelectionBlock();
        
        void SetPosition(const glm::ivec3 &position);
        glm::ivec3 GetPosition();

        void SetAdjacentPosition(const glm::ivec3 &position);
        glm::ivec3 GetAdjacentPosition();

        void SetMineProgress(float progress);
        float GetMineProgress();

        void SetActive(bool active);
        bool IsActive();

        void Render(const glm::mat4 &mvp_matrix);

    private:
        Mesh overlay_;
        glm::ivec3 position_;
        glm::ivec3 adjacent_position_;
        float mine_progress_ = 0;
        int mine_level_ = 0;
        bool active_;
};

class Moon
{
    public:
        Moon(int id, MoonSettings moon_settings);
        ~Moon();
        ChunkManager &GetChunkManager();
        EntityManager &GetEntityManager();
        Player *GetPlayer();
        glm::vec4 GetFogColor();
        int GetID();
        float GetLoadProgress();
        MoonSettings GetSettings();
        double GetWorldTime();
        glm::vec3 GetSunlightDirection();
        float GetSkyboxAngle();
        int GetLightPhase();
        void Update(double delta_time);
        void Render(const glm::mat4 &projection);

        static Moon *GetCurrentMoon();

    private:
        static Moon *current_moon_;

    private:
        int id_ = 0;
        int initial_chunk_count_ = 0;
        int current_light_phase_ = 1;
        double world_time_ = 0;
        double last_patch_update_ = 0;
        double accumulator_ = 0;
        bool skybox_reversed_ = false;
        float skybox_phase_ = LIGHT_CYCLE_OMEGA * SECONDS_PER_LIGHT_PHASE;
        glm::vec4 base_fog_color_; // This doesn't change for a given moon. Use GetFogColor() to get the correct color.
        Player *player_; // Avoids destructor call so EntityManager can destroy player
        SelectionBlock selection_block_;
        Skybox skybox_;
        MoonSettings settings_;
        ChunkManager chunk_manager_;
        EntityManager entity_manager_;

    private:
        void UpdateSelectionBlock(float delta_time);
};
