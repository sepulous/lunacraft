#ifndef MOON_H
#define MOON_H

#include <cstdint>

#include <glm/glm.hpp>

#include "moon_settings.h"
#include "chunk_manager.h"
#include "entity_manager.h"
#include "skybox.h"

class Moon
{
    private:
        int _id = 0;
        int _initial_chunk_count = 0;
        double _world_time;
        glm::vec4 _fog_color;
        Skybox _skybox;
        MoonSettings _settings;
        ChunkManager _chunk_manager;
        EntityManager _entity_manager;

    public:
        Moon(int id, MoonSettings moon_settings);
        ChunkManager &GetChunkManager();
        EntityManager &GetEntityManager();
        void UpdateWorldTime(double delta_time);
        void RenderSkybox(const glm::mat4& view_proj);
        glm::vec4 GetFogColor();
        int GetID();
        float GetLoadProgress();
        MoonSettings GetSettings();
        void Unload();
};

#endif
