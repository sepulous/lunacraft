
#include <glm/gtc/matrix_transform.hpp>

#include "viewport.h"

glm::dvec2 Viewport::_dimensions;
glm::dvec2 Viewport::_last_mouse_pos;
glm::mat4 Viewport::_proj_matrix;

void Viewport::SetDimensions(const glm::ivec2 &dimensions)
{
    _dimensions = dimensions;
    _proj_matrix = glm::perspective(glm::radians(45.0), (double)dimensions.x / (double)dimensions.y, 0.1, 500.0);
}

glm::ivec2 Viewport::GetDimensions()
{
    return _dimensions;
}

glm::mat4 Viewport::GetProjectionMatrix()
{
    return _proj_matrix;
}
