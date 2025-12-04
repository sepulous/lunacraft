#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Camera
{
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 forward;
    glm::vec3 right;
    float pitch = 0;
    float yaw = -90;
    float sensitivity = 0.05f;

    Camera()
    {
        position = glm::vec3(0.0f, 0.0f, 3.0f);
        forward = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 camera_direction = glm::normalize(position - glm::vec3(0, 0, 0));
        glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
        right = glm::normalize(glm::cross(world_up, camera_direction));
        up = glm::cross(camera_direction, right);
        position = glm::vec3(0.0f, 70.0f, 0.0f);
    }
};

#endif
