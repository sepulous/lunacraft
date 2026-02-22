#pragma once

#include <glm/glm.hpp>

#include "entity.h"
#include "camera.h"
#include "inventory.h"

struct PlayerData
{
    int health;
    int suit_status;
    glm::vec3 position;
    glm::vec2 camera_rotation; // {pitch, yaw}
    Inventory inventory;
};

class Player : public Entity
{
    private:
        Camera _camera;
        Inventory _inventory;
        bool _inventory_changed = false; // For UI updating. I don't love this system...
        glm::vec3 _input_direction;
        int _health = 100;
        int _suit_status = 100;
        bool _was_grounded = false;
        float _fall_time = 0;
        float _time_since_last_health_update = 0;
        float _time_since_last_suit_update = 0;

    public:
        Player();
        ~Player() = default;

        void Update(float delta_time) override;
        void FixedUpdate() override;

        PlayerData GetPlayerData();

        void UpdateCamera();
        Camera &GetCamera();

        void SetHealth(int health);
        int GetHealth();

        void SetSuitStatus(int suit_status);
        int GetSuitStatus();

        Inventory &GetInventory();
        void SetInventory(Inventory inventory);

        void SetCameraRotation(glm::vec2 rotation);
        glm::vec2 GetCameraRotation();
        void SetCameraSensitivity(float sensitivity);

        glm::vec3 GetForward();
        glm::vec3 GetRight();

    private:
        float GetMaxMoveSpeed();
        bool CanRegenSuit();
        float GetSuitRegenInterval();
};
