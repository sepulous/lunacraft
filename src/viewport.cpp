
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
