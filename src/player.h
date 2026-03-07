#pragma once

#include <glm/glm.hpp>

#include "entity.h"
#include "camera.h"
#include "inventory.h"
#include "mesh.h"
#include "sound_system.h"

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
        Camera camera_;
        Inventory inventory_;
        bool in_control_ = true;
        glm::vec3 input_direction_;
        int health_ = 100;
        int suit_status_ = 100;
        int jetpack_energy_ = 20;
        bool was_grounded_ = false;
        float fall_time_ = 0;
        bool is_flying_ = false;
        ActiveSound *drill_sound_ = nullptr;
        ActiveSound *jetpack_sound_ = nullptr;
        float time_since_last_health_update_ = 0;
        float time_since_last_suit_update_ = 0;
        float time_since_last_jetpack_update_ = 0;
        float time_since_started_flying_ = 0;
        ItemID last_held_sprite_ = ItemID::none; // So held item textures only update when necessary
        ItemID last_held_block_ = ItemID::none;  //
        Mesh arm_mesh_;
        Mesh pistol_base_mesh_;
        Mesh pistol_slide_mesh_;
        Mesh drill_base_mesh_;
        Mesh drill_bit_mesh_;
        Mesh sprite_mesh_;
        Mesh block_mesh_;

        // Animation variables
        float time_punching_ = 0;
        float time_walking_ = 0;
        float time_flying_ = 0;
        float time_drilling_ = 0;
        float arm_extent_ = 0;
        float arm_bob_ = 0;
        float arm_shake_ = 0;
        float drill_bit_extent_ = 0;
        float drill_bit_angular_speed_ = 0;
        float drill_bit_rotation_ = 0;

    public:
        Player();
        ~Player() = default;

        void Update(float delta_time) override;
        void FixedUpdate() override;
        void Render(const glm::mat4 &vp_matrix) override;

        PlayerData GetPlayerData();

        bool IsInControl();
        void EnableControl();
        void DisableControl();

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

    private:
        float GetMaxMoveSpeed();
        bool CanRegenSuit();
        float GetSuitRegenInterval();
};
