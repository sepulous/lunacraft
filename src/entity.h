#pragma once

#include "aabb.h"

class Entity
{
    protected:
        AABB _aabb;
        glm::vec3 _prev_position;
        glm::vec3 _next_position;
        glm::vec3 _position;
        glm::vec3 _velocity;
        bool _is_on_ice = false;
        bool _is_jumping = false;
        bool _is_grounded = false;

    public:
        virtual ~Entity() = default;
        virtual void Update() = 0;
        virtual void FixedUpdate() = 0;

        AABB &GetAABB() noexcept;

        void SetPosition(const glm::vec3& position) noexcept;
        glm::vec3 GetPosition() noexcept;

        void SetPrevPosition(const glm::vec3& prev_position) noexcept;
        glm::vec3 GetPrevPosition() noexcept;

        void SetNextPosition(const glm::vec3& next_position) noexcept;
        glm::vec3 GetNextPosition() noexcept;

        void SetVelocity(const glm::vec3& velocity) noexcept;
        glm::vec3 GetVelocity() noexcept;

        void SetIsOnIce(bool on_ice) noexcept;
        bool IsOnIce() noexcept;

        void SetJumping(bool value) noexcept;
        bool IsJumping() noexcept;

        void SetGrounded(bool value) noexcept;
        bool IsGrounded() noexcept;
};
