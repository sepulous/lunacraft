#ifndef MOON_H
#define MOON_H

#include <cstdint>

#include "chunk_manager.h"
#include "moon_settings.h"

class Moon
{
    private:
        int _id = 0;
        MoonSettings _settings;
        ChunkManager _chunk_manager;
        //PhysicsManager _physics_manager;

    public:
        Moon(int id, MoonSettings moon_settings);
        ChunkManager &GetChunkManager();
        int GetID();
        void Unload();
};

#endif
