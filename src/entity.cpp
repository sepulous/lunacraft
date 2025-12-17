
#include "entity.h"

AABB &Entity::GetAABB()
{
    return _aabb;
}

void Entity::SetPosition(glm::vec3 position)
{
    _position = position;
}

glm::vec3 Entity::GetPosition()
{
    return _position;
}

void Entity::SetPrevPosition(glm::vec3 prev_position)
{
    _prev_position = prev_position;
}

glm::vec3 Entity::GetPrevPosition()
{
    return _prev_position;
}

void Entity::SetNextPosition(glm::vec3 next_position)
{
    _next_position = next_position;
}

glm::vec3 Entity::GetNextPosition()
{
    return _next_position;
}

void Entity::SetVelocity(glm::vec3 velocity)
{
    _velocity = velocity;
}

glm::vec3 Entity::GetVelocity()
{
    return _velocity;
}

void Entity::SetInputDirection(glm::vec3 input_direction)
{
    _input_direction = input_direction;
}

glm::vec3 Entity::GetInputDirection()
{
    return _input_direction;
}

void Entity::SetJumping(bool value)
{
    _is_jumping = value;
}

bool Entity::IsJumping()
{
    return _is_jumping;
}

void Entity::SetGrounded(bool value)
{
    _is_grounded = value;
}

bool Entity::IsGrounded()
{
    return _is_grounded;
}
