#pragma once

#include <iostream>
#include <vector>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "entity.h"
#include "sound_system.h"
#include "constants.h"
#include "input.h"

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
        int _health = 100;
        int _suit_status = 100;
        bool _was_grounded = false;
        float _fall_time = 0;

    public:
        Player()
        {
            _position = glm::vec3(0.0f, 90.0f + 0.5f + 0.9f, 0.0f);
            _prev_position = _position;
            _next_position = _position;
            _velocity = glm::vec3(0);
            _aabb.center = _position;
            _aabb.extents = glm::vec3(0.4f, 0.9f, 0.4f);
            _camera.position = _position + glm::vec3(0, 0.9f, 0);
        }

        void Update() override
        {
            if (glm::length(_input_direction) > 0)
                _input_direction = glm::normalize(_input_direction);

            _camera.position = _position + glm::vec3(0, 0.9f, 0);
        }

        void FixedUpdate() override
        {
            _velocity.x = _move_speed * _input_direction.x;
            _velocity.z = _move_speed * _input_direction.z;

            if (_is_jumping && _is_grounded)
            {
                _velocity.y = 6.0f;
                _is_jumping = false;
            }

            if (!_was_grounded && _is_grounded)
            {
                if (_fall_time >= 1.0f)
                    SoundSystem::Play(SoundSystem::Sound::LAND);
                _fall_time = 0;
            }
            else if (!_was_grounded)
            {
                _fall_time += FIXED_DELTA_TIME;
            }
            _was_grounded = _is_grounded;
        }

        void UpdateCamera()
        {
            auto mouse_delta = Input::GetMouseDelta();
            _camera.yaw += mouse_delta.x * _camera.sensitivity;
            _camera.pitch += mouse_delta.y * _camera.sensitivity;
            _camera.pitch = glm::clamp(_camera.pitch, -89.8f, 89.8f);

            glm::vec3 direction;
            direction.x = cos(glm::radians(_camera.yaw)) * cos(glm::radians(_camera.pitch));
            direction.y = sin(glm::radians(_camera.pitch));
            direction.z = sin(glm::radians(_camera.yaw)) * cos(glm::radians(_camera.pitch));
            _camera.forward = glm::normalize(direction);
            _camera.right = glm::normalize(glm::cross(_camera.forward, _camera.up));
        }

        PlayerData GetPlayerData()
        {
            PlayerData player_data;
            player_data.health = _health;
            player_data.suit_status = _suit_status;
            player_data.position = _position;
            player_data.camera_rotation = GetCameraRotation();
            return player_data;
        }

        Camera &GetCamera() { return _camera; }

        void SetHealth(int health) { _health = health; }
        int GetHealth() { return _health; }

        void SetSuitStatus(int suit_status) { _suit_status = suit_status; }
        int GetSuitStatus() { return _suit_status; }

        void SetCameraRotation(glm::vec2 rotation) { _camera.pitch = rotation.x; _camera.yaw = rotation.y; }
        glm::vec2 GetCameraRotation() { return {_camera.pitch, _camera.yaw}; }

        void SetCameraSensitivity(float sensitivity) { _camera.SetSensitivity(sensitivity); }
};
