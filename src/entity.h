#pragma once

#include "aabb.h"

class Entity
{
    protected:
        AABB aabb_;
        glm::vec3 prev_position_;
        glm::vec3 next_position_;
        glm::vec3 position_;
        glm::vec3 velocity_;
        bool is_on_ice_ = false;
        bool is_jumping_ = false;
        bool is_grounded_ = false;

    public:
        virtual ~Entity() = default;
        virtual void Update(float delta_time) = 0;
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
