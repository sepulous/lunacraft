
#include <iostream>
#include <vector>

#include "sound_system.h"
#include "storage.h"

using Sound = SoundSystem::Sound;

std::vector<ActiveSound *> SoundSystem::active_sounds_;
std::unordered_map<Sound, Soundlib::Sound> SoundSystem::sound_map_;
float SoundSystem::sfx_volume_;
float SoundSystem::music_volume_;

void SoundSystem::Init()
{
    Soundlib::Init();

    sound_map_[Sound::SONG_1].LoadSound((Storage::SOUNDS / "theme1.mp3").string());
    sound_map_[Sound::SONG_2].LoadSound((Storage::SOUNDS / "theme2.mp3").string());
    sound_map_[Sound::SONG_3].LoadSound((Storage::SOUNDS / "theme3.mp3").string());
    sound_map_[Sound::SONG_4].LoadSound((Storage::SOUNDS / "theme4.mp3").string());
    sound_map_[Sound::SONG_5].LoadSound((Storage::SOUNDS / "theme5.mp3").string());
    sound_map_[Sound::BLOCK_BREAK].LoadSound((Storage::SOUNDS / "blockbreak.wav").string());
    sound_map_[Sound::BLOCK_PLACE].LoadSound((Storage::SOUNDS / "blockplace.wav").string());
    sound_map_[Sound::CRAFT].LoadSound((Storage::SOUNDS / "craft.wav").string());
    sound_map_[Sound::DING].LoadSound((Storage::SOUNDS / "ding.wav").string());
    sound_map_[Sound::DRILL].LoadSound((Storage::SOUNDS / "drill.wav").string());
    sound_map_[Sound::DRILL2].LoadSound((Storage::SOUNDS / "drill2.wav").string());
    sound_map_[Sound::DRILL3].LoadSound((Storage::SOUNDS / "drill3.wav").string());
    sound_map_[Sound::FRIENDLY_SUMMON].LoadSound((Storage::SOUNDS / "friendlysummon.wav").string());
    sound_map_[Sound::HURT].LoadSound((Storage::SOUNDS / "hurt.wav").string());
    sound_map_[Sound::JETPACK].LoadSound((Storage::SOUNDS / "jetpack.wav").string());
    sound_map_[Sound::JUMP].LoadSound((Storage::SOUNDS / "jump.wav").string());
    sound_map_[Sound::LAND].LoadSound((Storage::SOUNDS / "land.wav").string());
    sound_map_[Sound::LASER].LoadSound((Storage::SOUNDS / "lasergun.wav").string());
    sound_map_[Sound::MEDKIT].LoadSound((Storage::SOUNDS / "medkit.wav").string());
    sound_map_[Sound::PICKUP].LoadSound((Storage::SOUNDS / "pickup.wav").string());

    active_sounds_.reserve(ACTIVE_SOUND_LIMIT);
}

void SoundSystem::Exit()
{
    for (auto active_sound : active_sounds_)
    {
        active_sound->source->Stop();
        delete active_sound;
    }

    Soundlib::Exit();
}

void SoundSystem::Update(Options options)
{
    sfx_volume_ = options.sfx_volume;
    music_volume_ = options.music_volume;

    for (auto it = active_sounds_.begin(); it != active_sounds_.end(); )
    {
        auto active_sound = *it;

        if (active_sound->source->GetState() == Soundlib::SourceState::STOPPED) // Remove finished sounds
        {
            delete active_sound;
            it = active_sounds_.erase(it);
        }
        else
        {
            // Update volumes
            if (IsMusic(active_sound->sound_id))
                active_sound->source->SetGain(music_volume_);
            else
                active_sound->source->SetGain(sfx_volume_);

            // Update positions of global sounds
            if (active_sound->is_global)
                active_sound->source->SetPosition(Soundlib::GetListenerPosition());

            ++it;
        }
    }
}

// Play global sound
ActiveSound *SoundSystem::Play(Sound sound, bool loop)
{
    if (active_sounds_.size() == ACTIVE_SOUND_LIMIT)
        return nullptr;

    auto source = new Soundlib::SoundSource(sound_map_[sound]);
    source->SetPosition(Soundlib::GetListenerPosition());
    source->SetRolloffFactor(0);
    source->SetLooping(loop);
    source->SetGain(IsMusic(sound) ? music_volume_ : sfx_volume_);
    source->Play();

    auto active_sound = new ActiveSound{source, sound, true};
    active_sounds_.push_back(active_sound);
    return active_sound;
}

// Play positioned sound
ActiveSound *SoundSystem::PlayAt(Sound sound, glm::vec3 position, bool loop)
{
    if (active_sounds_.size() == ACTIVE_SOUND_LIMIT)
        return nullptr;

    auto source = new Soundlib::SoundSource(sound_map_[sound]);
    source->SetPosition({position.x, position.y, position.z});
    source->SetLooping(loop);
    source->SetGain(IsMusic(sound) ? music_volume_ : sfx_volume_);
    source->SetGain(1.0f);
    source->Play();
    
    auto active_sound = new ActiveSound{source, sound, true};
    active_sounds_.push_back(active_sound);
    return active_sound;
}

void SoundSystem::Stop(ActiveSound *active_sound)
{
    active_sound->source->Stop();
}

void SoundSystem::SetPlayerPosition(glm::vec3 position)
{
    Soundlib::SetListenerPosition({position.x, position.y, position.z});
}

void SoundSystem::SetPlayerOrientation(glm::vec3 forward, glm::vec3 up)
{
    Soundlib::SetListenerOrientation(
        {forward.x, forward.y, forward.z},
        {up.x, up.y, up.z}
    );
}

bool SoundSystem::IsMusic(Sound sound)
{
    return sound == Sound::SONG_1 || sound == Sound::SONG_2 || sound == Sound::SONG_3 || sound == Sound::SONG_4 || sound == Sound::SONG_5;
}
