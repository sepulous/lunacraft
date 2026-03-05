
#include <iostream>
#include <vector>

#include "sound_system.h"
#include "storage.h"

using Sound = SoundSystem::Sound;

typedef std::tuple<Sound, bool, std::unique_ptr<Soundlib::SoundSource>> ActiveSound;

std::vector<ActiveSound> SoundSystem::active_sounds_;
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
}

void SoundSystem::Exit()
{
    for (auto &[sound, is_global, source] : active_sounds_)
        source->Stop();
    active_sounds_.clear();

    Soundlib::Exit();
}

void SoundSystem::Update(Options options)
{
    sfx_volume_ = options.sfx_volume;
    music_volume_ = options.music_volume;

    for (auto it = active_sounds_.begin(); it != active_sounds_.end(); )
    {
        auto &[sound_id, is_global, sound_source] = *it;

        if (sound_source->GetState() == Soundlib::SourceState::STOPPED) // Remove finished sounds
        {
            it = active_sounds_.erase(it);
        }
        else
        {
            // Update volumes
            if (sound_id <= Sound::SONG_5)
                sound_source->SetGain(music_volume_);
            else
                sound_source->SetGain(sfx_volume_);

            // Update positions of global sounds
            if (is_global)
                sound_source->SetPosition(Soundlib::GetListenerPosition());

            ++it;
        }
    }
}

// Plays global Soundlib::Sound (without distance attenuation)
void SoundSystem::Play(Sound sound)
{
    auto source = std::make_unique<Soundlib::SoundSource>(sound_map_[sound]);
    source->SetPosition(Soundlib::GetListenerPosition());
    source->SetRolloffFactor(0);
    if (sound <= Sound::SONG_5)
        source->SetGain(music_volume_);
    else
        source->SetGain(sfx_volume_);
    source->Play();
    active_sounds_.emplace_back(sound, true, std::move(source));
}

// Plays positioned Soundlib::Sound (with distance attenuation)
void SoundSystem::PlayAt(Sound sound, glm::vec3 position)
{
    auto source = std::make_unique<Soundlib::SoundSource>(sound_map_[sound]);
    source->SetPosition({position.x, position.y, position.z});
    if (sound <= Sound::SONG_5)
        source->SetGain(music_volume_);
    else
        source->SetGain(sfx_volume_);
    source->Play();
    active_sounds_.emplace_back(sound, false, std::move(source));
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
