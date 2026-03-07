#pragma once

#include <vector>
#include <tuple>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>
#include <soundlib/soundlib.h>

#include "options.h"

#define ACTIVE_SOUND_LIMIT 1024

struct ActiveSound;

class SoundSystem
{
    public:
        enum class Sound
        {
            SONG_1,
            SONG_2,
            SONG_3,
            SONG_4,
            SONG_5,
            BLOCK_BREAK,
            BLOCK_PLACE,
            CRAFT,
            DING,
            DRILL,
            DRILL2,
            DRILL3,
            FRIENDLY_SUMMON,
            HURT,
            JETPACK,
            JUMP,
            LAND,
            LASER,
            MEDKIT,
            PICKUP
        };
        
    public:
        SoundSystem() = delete;
        static void Init();
        static void Exit();
        static void Update(Options options);
        static void Play(Sound sound);
        static void PlayAt(Sound sound, glm::vec3 position);
        static ActiveSound *PlayLooped(Sound sound);
        static ActiveSound *PlayLoopedAt(Sound sound, glm::vec3 position);
        static void SetPlayerPosition(glm::vec3 position);
        static void SetPlayerOrientation(glm::vec3 forward, glm::vec3 up);

    private:
        static std::vector<ActiveSound> active_sounds_;
        static std::unordered_map<Sound, Soundlib::Sound> sound_map_;
        static float sfx_volume_;
        static float music_volume_;
};

struct ActiveSound
{
    Soundlib::SoundSource *source;
    SoundSystem::Sound sound_id;
    bool is_global;
};
