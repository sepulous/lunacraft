#pragma once

#include "aabb.h"

enum class EntityType : uint8_t
{
    PLAYER,
    SLUG,
    MINILIGHT,
    DROPPED_ITEM,
    GREEN_MOB,
    BROWN_MOB,
    GIRAFFE,
    ASTRONAUT
};

class Entity
{
    friend class EntityManager;

    public:
        virtual ~Entity() = default;
        virtual void Update(float delta_time) = 0;
        virtual void FixedUpdate() = 0;
        virtual void Render(const glm::mat4 &vp_matrix) = 0;

        EntityType GetType() noexcept;

        AABB &GetAABB() noexcept;

        void SetPosition(const glm::vec3& position) noexcept;
        glm::vec3 GetPosition() noexcept;

        void SetPrevPosition(const glm::vec3& prev_position) noexcept;
        glm::vec3 GetPrevPosition() noexcept;

        void SetNextPosition(const glm::vec3& next_position) noexcept;
        glm::vec3 GetNextPosition() noexcept;

        void SetVelocity(const glm::vec3& velocity) noexcept;
        glm::vec3 GetVelocity() noexcept;

        void SetIsDead(bool value) noexcept;
        bool IsDead() noexcept;

        void SetIsOnIce(bool on_ice) noexcept;
        bool IsOnIce() noexcept;

        void SetJumping(bool value) noexcept;
        bool IsJumping() noexcept;

        void SetGrounded(bool value) noexcept;
        bool IsGrounded() noexcept;

        virtual void SetHealth(int health) noexcept;
        int GetHealth() noexcept;

        void SetCanBeDamaged(bool value) noexcept;
        bool CanBeDamaged() noexcept;

        bool IsDeathAnimationDone() noexcept;

    protected:
        size_t id_;
        EntityType type_;
        AABB aabb_;
        glm::vec3 prev_position_;
        glm::vec3 next_position_;
        glm::vec3 position_;
        glm::vec3 velocity_;
        int health_ = 100;
        float pain_time_ = 0;
        bool can_be_damaged_ = true;
        bool death_animation_done_ = false;
        bool is_dead_ = false;
        bool is_on_ice_ = false;
        bool is_jumping_ = false;
        bool is_grounded_ = false;

        void SetID(size_t id);
        size_t GetID();
};
