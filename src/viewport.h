#pragma once

#include <glm/glm.hpp>

class Viewport
{
public:
    static void SetDimensions(const glm::ivec2 &dimensions);
    static glm::ivec2 GetDimensions();
    static glm::mat4 GetProjectionMatrix();

private:
    static glm::dvec2 _dimensions;
    static glm::dvec2 _last_mouse_pos;
    static glm::mat4 _proj_matrix;
};
