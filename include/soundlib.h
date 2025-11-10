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

    enum DistanceModel : int
    {
        INVERSE_DISTANCE,
        INVERSE_DISTANCE_CLAMPED, // OpenAL default
        LINEAR_DISTANCE,
        LINEAR_DISTANCE_CLAMPED,
        EXPONENT_DISTANCE,
        EXPONENT_DISTANCE_CLAMPED
    };

    enum SourceState : int
    {
        INITIAL,
        PLAYING,
        PAUSED,
        STOPPED
    };

    class Sound
    {
        friend class SoundSource;

        private:
            unsigned m_buffer;

        public:
            ~Sound();
            Sound();
            Sound(const char*);
            void LoadSound(const char*);
    };

    class SoundSource
    {
        private:
            unsigned m_source;

        public:
            ~SoundSource();
            SoundSource();
            SoundSource(Sound);

            SourceState GetState();

            void SetSound(Sound);
            void Play();
            void Pause();
            void Stop();

            float GetGain();
            void SetGain(float);

            float GetMinGain();
            void SetMinGain(float);

            float GetMaxGain();
            void SetMaxGain(float);

            float GetMaxDistance();
            void SetMaxDistance(float);

            float GetRolloffFactor();
            void SetRolloffFactor(float);

            float GetConeInnerAngle();
            void SetConeInnerAngle(float);

            float GetConeOuterAngle();
            void SetConeOuterAngle(float);

            float GetConeOuterGain();
            void SetConeOuterGain(float);

            float GetPitch();
            void SetPitch(float);

            Vector3 GetPosition();
            void SetPosition(float, float, float);
            void SetPosition(Vector3);

            Vector3 GetVelocity();
            void SetVelocity(float, float, float);
            void SetVelocity(Vector3);

            Vector3 GetDirection();
            void SetDirection(float, float, float);
            void SetDirection(Vector3);

            bool IsLooping();
            void SetLooping(bool);
    };

    bool Init();
    bool Init(std::string);
    void Exit(); // This doesn't strictly need to be called at the end of the program, but some OpenAL implementations will complain if you don't
    std::vector<std::string> GetDeviceList();

    DistanceModel GetDistanceModel();
    void SetDistanceModel(DistanceModel);

    float GetDopplerFactor();
    void SetDopplerFactor(float);

    float GetSpeedOfSound();
    void SetSpeedOfSound(float);

    float GetListenerVolume();
    void SetListenerVolume(float);

    Vector3 GetListenerPosition();
    void SetListenerPosition(float, float, float);
    void SetListenerPosition(Vector3);

    Vector3 GetListenerVelocity();
    void SetListenerVelocity(float, float, float);
    void SetListenerVelocity(Vector3);

    Orientation GetListenerOrientation();
    void SetListenerOrientation(float, float, float, float, float, float);
    void SetListenerOrientation(Vector3, Vector3);
    void SetListenerOrientation(Orientation);
}
