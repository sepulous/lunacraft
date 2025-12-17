#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include <vector>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "entity.h"

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
        }

        void UpdateCamera(double x_pos, double y_pos, double x_offset, double y_offset)
        {
            _camera.yaw += x_offset * _camera.sensitivity;
            _camera.pitch += y_offset * _camera.sensitivity;
            _camera.pitch = glm::clamp(_camera.pitch, -89.8f, 89.8f);

            glm::vec3 direction;
            direction.x = cos(glm::radians(_camera.yaw)) * cos(glm::radians(_camera.pitch));
            direction.y = sin(glm::radians(_camera.pitch));
            direction.z = sin(glm::radians(_camera.yaw)) * cos(glm::radians(_camera.pitch));
            _camera.forward = glm::normalize(direction);
            _camera.right = glm::normalize(glm::cross(_camera.forward, _camera.up));
        }

        Camera GetCamera() { return _camera; }

        void SetHealth(int health) { _health = health; }
        int GetHealth() { return _health; }

        void SetSuitStatus(int suit_status) { _suit_status = suit_status; }
        int GetSuitStatus() { return _suit_status; }

        void SetCameraRotation(glm::vec2 rotation) { _camera.pitch = rotation.x; _camera.yaw = rotation.y; }
        glm::vec2 GetCameraRotation() { return {_camera.pitch, _camera.yaw}; }
};

#endif