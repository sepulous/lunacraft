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

        void SetActive(bool active);

        void Render(const glm::mat4 &mvp_matrix);

    private:
        Mesh _mesh;
        glm::ivec3 _position;
        bool _active;
};

class Moon
{
    private:
        static Moon *_current_moon;

    private:
        int _id = 0;
        int _initial_chunk_count = 0;
        int _current_light_phase = 1;
        double _world_time = 0;
        double _last_patch_update = 0;
        double _accumulator = 0;
        glm::vec4 _base_fog_color; // This doesn't change for a given moon. Use GetFogColor() to get the correct color.
        Player *_player; // Avoids destructor call so EntityManager can destroy player
        SelectionBlock _selection_block;
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
        double GetWorldTime();
        glm::vec3 GetSunlightDirection();
        void Update(double delta_time);
        void Render(const glm::mat4 &projection);

        static Moon *GetCurrentMoon();
};
