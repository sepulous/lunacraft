#pragma once

#include <vector>
#include <tuple>
#include <unordered_map>

#include <glm/glm.hpp>

#include "soundlib.h"
#include "options.h"

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
        static void SetPlayerPosition(glm::vec3 position);
        static void SetPlayerOrientation(glm::vec3 forward, glm::vec3 up);

    private:
        static std::vector<std::tuple<Sound, Soundlib::SoundSource *>> _active_sounds;
        static std::unordered_map<Sound, Soundlib::Sound> _sound_map;
        static float _sfx_volume;
        static float _music_volume;
};
