
#include <iostream>
#include <vector>

#include "sound_system.h"
#include "storage.h"

using Sound = SoundSystem::Sound;

std::vector<std::tuple<Sound, Soundlib::SoundSource *>> SoundSystem::_active_sounds;
std::unordered_map<Sound, Soundlib::Sound> SoundSystem::_sound_map;

void SoundSystem::Init()
{
    Soundlib::Init();

    _sound_map[Sound::SONG_1].LoadSound((Storage::ASSET_DIR / "sounds" / "theme1.mp3").string());
    _sound_map[Sound::SONG_2].LoadSound((Storage::ASSET_DIR / "sounds" / "theme2.mp3").string());
    _sound_map[Sound::SONG_3].LoadSound((Storage::ASSET_DIR / "sounds" / "theme3.mp3").string());
    _sound_map[Sound::SONG_4].LoadSound((Storage::ASSET_DIR / "sounds" / "theme4.mp3").string());
    _sound_map[Sound::SONG_5].LoadSound((Storage::ASSET_DIR / "sounds" / "theme5.mp3").string());
    _sound_map[Sound::BLOCK_BREAK].LoadSound((Storage::ASSET_DIR / "sounds" / "blockbreak.wav").string());
    _sound_map[Sound::BLOCK_PLACE].LoadSound((Storage::ASSET_DIR / "sounds" / "blockplace.wav").string());
    _sound_map[Sound::CRAFT].LoadSound((Storage::ASSET_DIR / "sounds" / "craft.wav").string());
    _sound_map[Sound::DING].LoadSound((Storage::ASSET_DIR / "sounds" / "ding.wav").string());
    _sound_map[Sound::DRILL].LoadSound((Storage::ASSET_DIR / "sounds" / "drill.wav").string());
    _sound_map[Sound::DRILL2].LoadSound((Storage::ASSET_DIR / "sounds" / "drill2.wav").string());
    _sound_map[Sound::DRILL3].LoadSound((Storage::ASSET_DIR / "sounds" / "drill3.wav").string());
    _sound_map[Sound::FRIENDLY_SUMMON].LoadSound((Storage::ASSET_DIR / "sounds" / "friendlysummon.wav").string());
    _sound_map[Sound::HURT].LoadSound((Storage::ASSET_DIR / "sounds" / "hurt.wav").string());
    _sound_map[Sound::JETPACK].LoadSound((Storage::ASSET_DIR / "sounds" / "jetpack.wav").string());
    _sound_map[Sound::JUMP].LoadSound((Storage::ASSET_DIR / "sounds" / "jump.wav").string());
    _sound_map[Sound::LAND].LoadSound((Storage::ASSET_DIR / "sounds" / "land.wav").string());
    _sound_map[Sound::LASER].LoadSound((Storage::ASSET_DIR / "sounds" / "lasergun.wav").string());
    _sound_map[Sound::MEDKIT].LoadSound((Storage::ASSET_DIR / "sounds" / "medkit.wav").string());
    _sound_map[Sound::PICKUP].LoadSound((Storage::ASSET_DIR / "sounds" / "pickup.wav").string());
}

void SoundSystem::Exit()
{
    Soundlib::Exit();
}

void SoundSystem::Update(Options options)
{
    // Remove finished sounds
    std::erase_if(_active_sounds, [](std::tuple<Sound, Soundlib::SoundSource *> sound) {
        return std::get<1>(sound)->GetState() == Soundlib::SourceState::STOPPED;
    });

    // Update volumes
    for (std::tuple<Sound, Soundlib::SoundSource *>& sound : _active_sounds)
    {
        Sound sound_id = std::get<0>(sound);
        if (sound_id == Sound::SONG_1 || sound_id == Sound::SONG_2 || sound_id == Sound::SONG_3 || sound_id == Sound::SONG_4 || sound_id == Sound::SONG_5)
            std::get<1>(sound)->SetGain(options.music_volume);
        else
            std::get<1>(sound)->SetGain(options.sfx_volume);
    }
}

// Plays global Soundlib::Sound (without distance attenuation)
void SoundSystem::Play(Sound sound)
{
    Soundlib::SoundSource *source = new Soundlib::SoundSource(_sound_map[sound]);
    source->SetRolloffFactor(0);
    source->Play();
    _active_sounds.emplace_back(sound, source);
}

// Plays positioned Soundlib::Sound (with distance attenuation)
void SoundSystem::PlayAt(Sound sound, glm::vec3 position)
{
    Soundlib::SoundSource *source = new Soundlib::SoundSource(_sound_map[sound]);
    source->SetPosition({position.x, position.y, position.z});
    source->Play();
    _active_sounds.emplace_back(sound, source);
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
