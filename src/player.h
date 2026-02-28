#pragma once

#include <glm/glm.hpp>

#include "entity.h"
#include "camera.h"
#include "inventory.h"
#include "mesh.h"

struct PlayerData
{
    int health;
    int suit_status;
    int jetpack_energy;
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
        int _jetpack_energy = 20;
        bool _was_grounded = false;
        float _fall_time = 0;
        bool _is_flying = false;
        float _time_punching = 0;
        float _time_since_last_health_update = 0;
        float _time_since_last_suit_update = 0;
        float _time_since_last_jetpack_update = 0;
        float _time_since_started_flying = 0;
        ItemID _last_held_sprite = ItemID::none; // So held item textures only update when necessary
        ItemID _last_held_block = ItemID::none;  //
        Mesh _arm_mesh;
        Mesh _pistol_base_mesh;
        Mesh _pistol_slide_mesh;
        Mesh _drill_base_mesh;
        Mesh _drill_bit_mesh;
        Mesh _sprite_mesh;
        Mesh _block_mesh;

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

        void SetJetpackEnergy(int energy);
        int GetJetpackEnergy();
        int GetMaxJetpackEnergy();

        Inventory &GetInventory();
        void SetInventory(Inventory inventory);

        void SetCameraRotation(glm::vec2 rotation);
        glm::vec2 GetCameraRotation();
        void SetCameraSensitivity(float sensitivity);

        glm::vec3 GetForward();
        glm::vec3 GetRight();

        void RenderArm(const glm::mat4 &vp_matrix);

    private:
        float GetMaxMoveSpeed();
        bool CanRegenSuit();
        float GetSuitRegenInterval();
};
