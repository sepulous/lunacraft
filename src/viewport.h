#pragma once

#include <glm/glm.hpp>

class Viewport
{
public:
    static void SetDimensions(const glm::ivec2 &dimensions);
    static glm::ivec2 GetDimensions();

    static void SetLastMousePosition(const glm::dvec2 &position);
    static glm::dvec2 GetLastMousePosition();

private:
    static glm::dvec2 _dimensions;
    static glm::dvec2 _last_mouse_pos;
};
