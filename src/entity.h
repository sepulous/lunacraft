#ifndef ENTITY_H
#define ENTITY_H

#include "aabb.h"

class Entity
{
    protected:
        AABB _aabb;
        glm::vec3 _prev_position;
        glm::vec3 _next_position;
        glm::vec3 _position;
        glm::vec3 _velocity;
        glm::vec3 _input_direction;
        float _move_speed = 8.0f;
        bool _is_jumping = false;
        bool _is_grounded = false;

    public:
        virtual void Update() = 0;
        virtual void FixedUpdate() = 0;

        AABB &GetAABB();

        void SetPosition(glm::vec3 position);
        glm::vec3 GetPosition();

        void SetPrevPosition(glm::vec3 prev_position);
        glm::vec3 GetPrevPosition();

        void SetNextPosition(glm::vec3 next_position);
        glm::vec3 GetNextPosition();

        void SetVelocity(glm::vec3 velocity);
        glm::vec3 GetVelocity();

        void SetInputDirection(glm::vec3 input_direction);
        glm::vec3 GetInputDirection();

        void SetJumping(bool value);
        bool IsJumping();

        void SetGrounded(bool value);
        bool IsGrounded();
};

#endif
