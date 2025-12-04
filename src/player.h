#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include <vector>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "aabb.h"

struct PlayerData
{
    int health;
    int suit_status;
    glm::vec3 position;
    glm::vec2 camera_rotation; // {pitch, yaw}
};

class Player
{
    public:
        Camera camera;
        AABB aabb;
        int health = 100;
        int suit_status = 100;
        glm::vec3 prev_position;
        glm::vec3 next_position;
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 input_direction;
        float walk_speed = 8.0f;
        bool is_jumping = false;
        bool is_grounded = false;

        Player()
        {
            position = glm::vec3(0.0f, 70.0f + 0.5f + 0.9f, 0.0f);
            prev_position = position;
            next_position = position;
            velocity = glm::vec3(0);
            aabb.center = position;
            aabb.extents = glm::vec3(0.4f, 0.9f, 0.4f);
            camera.position = position + glm::vec3(0, 0.9f, 0);
        }

        void Update()
        {
            if (glm::length(input_direction) > 0)
                input_direction = glm::normalize(input_direction);

            camera.position = position + glm::vec3(0, 0.9f, 0);
        }

        void FixedUpdate()
        {
            velocity.x = walk_speed * input_direction.x;
            velocity.z = walk_speed * input_direction.z;

            if (is_jumping && is_grounded)
            {
                velocity.y = 6.0f;
                is_jumping = false;
            }
        }
};

#endif