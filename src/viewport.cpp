
#include <glm/gtc/matrix_transform.hpp>

#include "viewport.h"

glm::dvec2 Viewport::dimensions_;
glm::dvec2 Viewport::last_mouse_pos_;
glm::mat4 Viewport::proj_matrix_;

void Viewport::SetDimensions(const glm::ivec2 &dimensions)
{
    dimensions_ = dimensions;
    proj_matrix_ = glm::perspective(glm::radians(45.0), (double)dimensions.x / (double)dimensions.y, 0.1, 500.0);
}

glm::ivec2 Viewport::GetDimensions()
{
    return dimensions_;
}

glm::mat4 Viewport::GetProjectionMatrix()
{
    return proj_matrix_;
}
