
#include "viewport.h"

glm::dvec2 Viewport::_dimensions;
glm::dvec2 Viewport::_last_mouse_pos;

void Viewport::SetDimensions(const glm::ivec2 &dimensions)
{
    _dimensions = dimensions;
}

glm::ivec2 Viewport::GetDimensions()
{
    return _dimensions;
}

void Viewport::SetLastMousePosition(const glm::dvec2 &position)
{
    _last_mouse_pos = position;
}

glm::dvec2 Viewport::GetLastMousePosition()
{
    return _last_mouse_pos;
}
