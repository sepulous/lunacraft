
#include "entity.h"

EntityType Entity::GetType() noexcept
{
    return type_;
}

AABB &Entity::GetAABB() noexcept
{
    return aabb_;
}

void Entity::SetPosition(const glm::vec3& position) noexcept
{
    position_ = position;
}

glm::vec3 Entity::GetPosition() noexcept
{
    return position_;
}

void Entity::SetPrevPosition(const glm::vec3& prev_position) noexcept
{
    prev_position_ = prev_position;
}

glm::vec3 Entity::GetPrevPosition() noexcept
{
    return prev_position_;
}

void Entity::SetNextPosition(const glm::vec3& next_position) noexcept
{
    next_position_ = next_position;
}

glm::vec3 Entity::GetNextPosition() noexcept
{
    return next_position_;
}

void Entity::SetVelocity(const glm::vec3& velocity) noexcept
{
    velocity_ = velocity;
}

glm::vec3 Entity::GetVelocity() noexcept
{
    return velocity_;
}

void Entity::SetIsDead(bool value) noexcept
{
    is_dead_ = value;
}

bool Entity::IsDead() noexcept
{
    return is_dead_;
}

void Entity::SetIsOnIce(bool on_ice) noexcept
{
    is_on_ice_ = on_ice;
}

bool Entity::IsOnIce() noexcept
{
    return is_on_ice_;
}

void Entity::SetJumping(bool value) noexcept
{
    is_jumping_ = value;
}

bool Entity::IsJumping() noexcept
{
    return is_jumping_;
}

void Entity::SetGrounded(bool value) noexcept
{
    is_grounded_ = value;
}

bool Entity::IsGrounded() noexcept
{
    return is_grounded_;
}
