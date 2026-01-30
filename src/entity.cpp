
#include "entity.h"

AABB &Entity::GetAABB() noexcept
{
    return _aabb;
}

void Entity::SetPosition(const glm::vec3& position) noexcept
{
    _position = position;
}

glm::vec3 Entity::GetPosition() noexcept
{
    return _position;
}

void Entity::SetPrevPosition(const glm::vec3& prev_position) noexcept
{
    _prev_position = prev_position;
}

glm::vec3 Entity::GetPrevPosition() noexcept
{
    return _prev_position;
}

void Entity::SetNextPosition(const glm::vec3& next_position) noexcept
{
    _next_position = next_position;
}

glm::vec3 Entity::GetNextPosition() noexcept
{
    return _next_position;
}

void Entity::SetVelocity(const glm::vec3& velocity) noexcept
{
    _velocity = velocity;
}

glm::vec3 Entity::GetVelocity() noexcept
{
    return _velocity;
}

void Entity::SetIsOnIce(bool on_ice) noexcept
{
    _is_on_ice = on_ice;
}

bool Entity::IsOnIce() noexcept
{
    return _is_on_ice;
}

void Entity::SetJumping(bool value) noexcept
{
    _is_jumping = value;
}

bool Entity::IsJumping() noexcept
{
    return _is_jumping;
}

void Entity::SetGrounded(bool value) noexcept
{
    _is_grounded = value;
}

bool Entity::IsGrounded() noexcept
{
    return _is_grounded;
}
