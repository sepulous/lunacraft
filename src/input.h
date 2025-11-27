#ifndef INPUT_H
#define INPUT_H

#include <glm/glm.hpp>

struct MouseState
{
    glm::dvec2 position; // In virtual coordinates
    bool left_clicked = false; // Only true on the first frame the mouse is clicked
    bool left_held = false;
};

#endif
