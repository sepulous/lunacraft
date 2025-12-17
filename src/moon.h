#ifndef MOON_H
#define MOON_H

#include <cstdint>

#include "moon_settings.h"
#include "chunk_manager.h"
#include "entity_manager.h"

class Moon
{
    private:
        int _id = 0;
        MoonSettings _settings;
        ChunkManager _chunk_manager;
        EntityManager _entity_manager;

    public:
        Moon(int id, MoonSettings moon_settings);
        ChunkManager &GetChunkManager();
        EntityManager &GetEntityManager();
        int GetID();
        void Unload();
};

#endif
