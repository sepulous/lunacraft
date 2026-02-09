
#include <iostream>
#include <vector>

#include "sound_system.h"
#include "storage.h"

using Sound = SoundSystem::Sound;

typedef std::tuple<Sound, bool, std::unique_ptr<Soundlib::SoundSource>> ActiveSound;

std::vector<ActiveSound> SoundSystem::_active_sounds;
std::unordered_map<Sound, Soundlib::Sound> SoundSystem::_sound_map;
float SoundSystem::_sfx_volume;
float SoundSystem::_music_volume;

void SoundSystem::Init()
{
    Soundlib::Init();

    _sound_map[Sound::SONG_1].LoadSound((Storage::SOUNDS / "theme1.mp3").string());
    _sound_map[Sound::SONG_2].LoadSound((Storage::SOUNDS / "theme2.mp3").string());
    _sound_map[Sound::SONG_3].LoadSound((Storage::SOUNDS / "theme3.mp3").string());
    _sound_map[Sound::SONG_4].LoadSound((Storage::SOUNDS / "theme4.mp3").string());
    _sound_map[Sound::SONG_5].LoadSound((Storage::SOUNDS / "theme5.mp3").string());
    _sound_map[Sound::BLOCK_BREAK].LoadSound((Storage::SOUNDS / "blockbreak.wav").string());
    _sound_map[Sound::BLOCK_PLACE].LoadSound((Storage::SOUNDS / "blockplace.wav").string());
    _sound_map[Sound::CRAFT].LoadSound((Storage::SOUNDS / "craft.wav").string());
    _sound_map[Sound::DING].LoadSound((Storage::SOUNDS / "ding.wav").string());
    _sound_map[Sound::DRILL].LoadSound((Storage::SOUNDS / "drill.wav").string());
    _sound_map[Sound::DRILL2].LoadSound((Storage::SOUNDS / "drill2.wav").string());
    _sound_map[Sound::DRILL3].LoadSound((Storage::SOUNDS / "drill3.wav").string());
    _sound_map[Sound::FRIENDLY_SUMMON].LoadSound((Storage::SOUNDS / "friendlysummon.wav").string());
    _sound_map[Sound::HURT].LoadSound((Storage::SOUNDS / "hurt.wav").string());
    _sound_map[Sound::JETPACK].LoadSound((Storage::SOUNDS / "jetpack.wav").string());
    _sound_map[Sound::JUMP].LoadSound((Storage::SOUNDS / "jump.wav").string());
    _sound_map[Sound::LAND].LoadSound((Storage::SOUNDS / "land.wav").string());
    _sound_map[Sound::LASER].LoadSound((Storage::SOUNDS / "lasergun.wav").string());
    _sound_map[Sound::MEDKIT].LoadSound((Storage::SOUNDS / "medkit.wav").string());
    _sound_map[Sound::PICKUP].LoadSound((Storage::SOUNDS / "pickup.wav").string());
}

void SoundSystem::Exit()
{
    for (auto &[sound, is_global, source] : _active_sounds)
        source->Stop();
    _active_sounds.clear();

    Soundlib::Exit();
}

void SoundSystem::Update(Options options)
{
    _sfx_volume = options.sfx_volume;
    _music_volume = options.music_volume;

    for (auto it = _active_sounds.begin(); it != _active_sounds.end(); )
    {
        auto &[sound_id, is_global, sound_source] = *it;

        if (sound_source->GetState() == Soundlib::SourceState::STOPPED) // Remove finished sounds
        {
            it = _active_sounds.erase(it);
        }
        else
        {
            // Update volumes
            if (sound_id <= Sound::SONG_5)
                sound_source->SetGain(_music_volume);
            else
                sound_source->SetGain(_sfx_volume);

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
    auto source = std::make_unique<Soundlib::SoundSource>(_sound_map[sound]);
    source->SetPosition(Soundlib::GetListenerPosition());
    source->SetRolloffFactor(0);
    if (sound <= Sound::SONG_5)
        source->SetGain(_music_volume);
    else
        source->SetGain(_sfx_volume);
    source->Play();
    _active_sounds.emplace_back(sound, true, std::move(source));
}

// Plays positioned Soundlib::Sound (with distance attenuation)
void SoundSystem::PlayAt(Sound sound, glm::vec3 position)
{
    auto source = std::make_unique<Soundlib::SoundSource>(_sound_map[sound]);
    source->SetPosition({position.x, position.y, position.z});
    if (sound <= Sound::SONG_5)
        source->SetGain(_music_volume);
    else
        source->SetGain(_sfx_volume);
    source->Play();
    _active_sounds.emplace_back(sound, false, std::move(source));
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
