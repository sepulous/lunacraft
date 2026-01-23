#pragma once

#include <cstdint>

#include <glm/glm.hpp>

#include "moon_settings.h"
#include "chunk_manager.h"
#include "entity_manager.h"
#include "skybox.h"
#include "player.h"

class Moon
{
    private:
        int _id = 0;
        int _initial_chunk_count = 0;
        double _world_time = 0;
        double _accumulator = 0;
        glm::vec4 _base_fog_color; // This doesn't change for a given moon. Use GetFogColor() to get the correct color.
        Player *_player; // Avoids destructor call so EntityManager can destroy player
        Skybox _skybox;
        MoonSettings _settings;
        ChunkManager _chunk_manager;
        EntityManager _entity_manager;

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
        void Update(double delta_time, int old_render_distance);
        void Render(glm::mat4 projection);
};
