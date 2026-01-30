#pragma once

#include <glm/glm.hpp>

#include "entity.h"
#include "camera.h"

struct PlayerData
{
    int health;
    int suit_status;
    glm::vec3 position;
    glm::vec2 camera_rotation; // {pitch, yaw}
};

class Player : public Entity
{
    private:
        Camera _camera;
        glm::vec3 _input_direction;
        int _health = 100;
        int _suit_status = 100;
        bool _on_ice = false;
        bool _was_grounded = false;
        float _fall_time = 0;

    public:
        Player();
        ~Player() = default;

        void Update() override;
        void FixedUpdate() override;

        PlayerData GetPlayerData();

        void UpdateCamera();
        Camera &GetCamera();

        void SetHealth(int health);
        int GetHealth();

        void SetSuitStatus(int suit_status);
        int GetSuitStatus();

        void SetCameraRotation(glm::vec2 rotation);
        glm::vec2 GetCameraRotation();
        void SetCameraSensitivity(float sensitivity);

        glm::vec3 GetForward();
        glm::vec3 GetRight();
};
