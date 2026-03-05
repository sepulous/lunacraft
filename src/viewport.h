#pragma once

#include <glm/glm.hpp>

class Viewport
{
public:
    static void SetDimensions(const glm::ivec2 &dimensions);
    static glm::ivec2 GetDimensions();
    static glm::mat4 GetProjectionMatrix();

private:
    static glm::dvec2 dimensions_;
    static glm::dvec2 last_mouse_pos_;
    static glm::mat4 proj_matrix_;
};
