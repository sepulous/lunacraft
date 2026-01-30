
#include "player.h"
#include "constants.h"
#include "sound_system.h"
#include "input.h"

Player::Player()
{
    _position = glm::vec3(CHUNK_SIZE / 2.0f, 90.0f + 0.5f + 0.9f, CHUNK_SIZE / 2.0f);
    _prev_position = _position;
    _next_position = _position;
    _velocity = glm::vec3(0);
    _aabb.center = _position;
    _aabb.extents = glm::vec3(0.4f, 0.9f, 0.4f);
    _camera.position = _position + glm::vec3(0, 0.9f, 0);
}

void Player::Update()
{
    _input_direction = glm::vec3(0);

    auto forward = GetForward();
    auto right = GetRight();
    if (Input::IsKeyHeld(GLFW_KEY_W))
        _input_direction += forward;
    if (Input::IsKeyHeld(GLFW_KEY_S))
        _input_direction -= forward;
    if (Input::IsKeyHeld(GLFW_KEY_A))
        _input_direction -= right;
    if (Input::IsKeyHeld(GLFW_KEY_D))
        _input_direction += right;
    if (Input::IsKeyHeld(GLFW_KEY_SPACE) && _is_grounded)
        _is_jumping = true;

    if (glm::length(_input_direction) > 0)
        _input_direction = glm::normalize(_input_direction);

    _camera.position = _position + glm::vec3(0, 0.9f, 0);
}

void Player::FixedUpdate()
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
        if (_fall_time >= 2.0f)
            SoundSystem::Play(SoundSystem::Sound::LAND);
        _fall_time = 0;
    }
    else if (!_was_grounded)
    {
        _fall_time += FIXED_DELTA_TIME;
    }
    _was_grounded = _is_grounded;
}

void Player::UpdateCamera()
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

PlayerData Player::GetPlayerData()
{
    PlayerData player_data;
    player_data.health = _health;
    player_data.suit_status = _suit_status;
    player_data.position = _position;
    player_data.camera_rotation = GetCameraRotation();
    return player_data;
}

Camera &Player::GetCamera()
{
    return _camera;
}

void Player::SetHealth(int health)
{
    _health = health;
}

int Player::GetHealth()
{
    return _health;
}

void Player::SetSuitStatus(int suit_status)
{
    _suit_status = suit_status;
}

int Player::GetSuitStatus()
{
    return _suit_status;
}

void Player::SetCameraRotation(glm::vec2 rotation)
{
    _camera.pitch = rotation.x;
    _camera.yaw = rotation.y;
}

glm::vec2 Player::GetCameraRotation()
{
    return {_camera.pitch, _camera.yaw};
}

void Player::SetCameraSensitivity(float sensitivity)
{
    _camera.SetSensitivity(sensitivity);
}

glm::vec3 Player::GetForward()
{
    return {
        glm::cos(glm::radians(_camera.yaw)),
        0,
        glm::sin(glm::radians(_camera.yaw))
    };
}

glm::vec3 Player::GetRight()
{
    return glm::cross(GetForward(), glm::vec3{0, 1, 0});
}
