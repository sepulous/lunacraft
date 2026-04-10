#pragma once

#include <vector>
#include <string>

namespace Soundlib
{
    struct Vector3
    {
        float x, y, z;
    };

    struct Orientation
    {
        Vector3 forward; // Direction listener's face points
        Vector3 up;      // Direction listener's head points
    };

    enum class AttenuationModel : int
    {
        INVERSE_DISTANCE,
        INVERSE_DISTANCE_CLAMPED, // OpenAL default
        LINEAR_DISTANCE,
        LINEAR_DISTANCE_CLAMPED,
        EXPONENT_DISTANCE,
        EXPONENT_DISTANCE_CLAMPED
    };

    enum class SourceState : int
    {
        INITIAL,
        PLAYING,
        PAUSED,
        STOPPED
    };

    enum class SoundFormat : int
    {
        MONO8,
        STEREO8,
        MONO16,
        STEREO16
    };

    enum class Error
    {
        NONE,
        SOURCE_CREATE_FAIL,
        SOURCE_BIND_FAIL,
        BUFFER_CREATE_FAIL,
        BUFFER_UPLOAD_FAIL,
        FILE_OPEN_FAIL,
        FILE_READ_FAIL
    };

    class Sound
    {
        friend class SoundSource;

        public:
            ~Sound();
            Sound();
            Sound(const std::string&);
            Sound(const std::string&, SoundFormat, float); // Only for RAW audio

            Sound(const Sound&) = delete;
            Sound& operator=(const Sound&) = delete;

            Sound(Sound&&) = delete;
            Sound& operator=(Sound&&) = delete;

            Error GetError() noexcept;

            void LoadSound(const std::string&);
            void LoadSoundRaw(const std::string&, SoundFormat, float);

        private:
            unsigned buffer_;
            Error error_ = Error::NONE;
    };

    class SoundSource
    {
        public:
            ~SoundSource();
            SoundSource();
            SoundSource(const Sound&);

            SoundSource(const SoundSource&) = delete;
            SoundSource& operator=(const SoundSource&) = delete;

            SoundSource(SoundSource&&) = delete;
            SoundSource& operator=(SoundSource&&) = delete;

            Error GetError() noexcept;
            SourceState GetState() noexcept;

            void SetSound(const Sound&) noexcept;
            void Play() noexcept;
            void Pause() noexcept;
            void Stop() noexcept;

            float GetGain() noexcept;
            void SetGain(float) noexcept;

            float GetMinGain() noexcept;
            void SetMinGain(float) noexcept;

            float GetMaxGain() noexcept;
            void SetMaxGain(float) noexcept;

            float GetMaxDistance() noexcept;
            void SetMaxDistance(float) noexcept;

            float GetReferenceDistance() noexcept;
            void SetReferenceDistance(float) noexcept;

            float GetRolloffFactor() noexcept;
            void SetRolloffFactor(float) noexcept;

            float GetConeInnerAngle() noexcept;
            void SetConeInnerAngle(float) noexcept;

            float GetConeOuterAngle() noexcept;
            void SetConeOuterAngle(float) noexcept;

            float GetConeOuterGain() noexcept;
            void SetConeOuterGain(float) noexcept;

            float GetPitch() noexcept;
            void SetPitch(float) noexcept;

            Vector3 GetPosition() noexcept;
            void SetPosition(float, float, float) noexcept;
            void SetPosition(Vector3) noexcept;

            Vector3 GetVelocity() noexcept;
            void SetVelocity(float, float, float) noexcept;
            void SetVelocity(Vector3) noexcept;

            Vector3 GetDirection() noexcept;
            void SetDirection(float, float, float) noexcept;
            void SetDirection(Vector3) noexcept;

            bool IsLooping() noexcept;
            void SetLooping(bool) noexcept;

        private:
            unsigned source_;
            Error error_ = Error::NONE;
    };

    bool Init();
    bool Init(const std::string&);
    void Exit();
    std::vector<std::string> GetDeviceList();

    AttenuationModel GetAttenuationModel() noexcept;
    void SetAttenuationModel(AttenuationModel) noexcept;

    float GetDopplerFactor() noexcept;
    void SetDopplerFactor(float) noexcept;

    float GetSpeedOfSound() noexcept;
    void SetSpeedOfSound(float) noexcept;

    float GetListenerVolume() noexcept;
    void SetListenerVolume(float) noexcept;

    Vector3 GetListenerPosition() noexcept;
    void SetListenerPosition(float, float, float) noexcept;
    void SetListenerPosition(Vector3) noexcept;

    Vector3 GetListenerVelocity() noexcept;
    void SetListenerVelocity(float, float, float) noexcept;
    void SetListenerVelocity(Vector3) noexcept;

    Orientation GetListenerOrientation() noexcept;
    void SetListenerOrientation(float, float, float, float, float, float) noexcept;
    void SetListenerOrientation(Vector3, Vector3) noexcept;
    void SetListenerOrientation(Orientation) noexcept;
}
