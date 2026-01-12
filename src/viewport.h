#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <glm/glm.hpp>

struct Viewport
{
    glm::dvec2 dimensions = {1280.0, 720.0};
    glm::dvec2 last_mouse_pos = {1280.0 / 2.0, 720.0 / 2.0};
    glm::mat4 ui_virtual_to_window = glm::mat4(1.0);
};

#endif
