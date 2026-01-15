
#include "input.h"
#include "viewport.h"

//
// State
//

ButtonState  Input::_keyboard_state[GLFW_KEY_LAST + 1];
ButtonState  Input::_mouse_state[GLFW_MOUSE_BUTTON_LAST + 1];
glm::dvec2   Input::_mouse_position;
glm::dvec2   Input::_mouse_delta;
int          Input::_mouse_scroll;
unsigned int Input::_last_char_input;

//
// API
//

void Input::BeginFrame()
{
    for (auto &k : _keyboard_state)
    {
        k.pressed  = false;
        k.released = false;
    }

    for (auto &k : _mouse_state)
    {
        k.pressed  = false;
        k.released = false;
    }

    _last_char_input = 0;
    _mouse_scroll = 0;
    _mouse_delta = glm::dvec2(0);
}

bool Input::IsKeyHeld(int key)
{
    return _keyboard_state[key].held;
}

bool Input::IsKeyPressed(int key)
{
    return _keyboard_state[key].pressed;
}

bool Input::IsKeyReleased(int key)
{
    return _keyboard_state[key].released;
}

bool Input::IsMouseButtonHeld(int button)
{
    return _mouse_state[button].held;
}

bool Input::IsMouseButtonPressed(int button)
{
    return _mouse_state[button].pressed;
}

bool Input::IsMouseButtonReleased(int button)
{
    return _mouse_state[button].released;
}

glm::dvec2 Input::GetMousePosition()
{
    return _mouse_position;
}

glm::dvec2 Input::GetVirtualMousePosition(const glm::mat4 &virtual_to_window)
{
    glm::vec4 virtual_mouse = glm::inverse(virtual_to_window) * glm::vec4(_mouse_position.x, Viewport::GetDimensions().y - _mouse_position.y, 0.0f, 1.0f);
    return {virtual_mouse.x, virtual_mouse.y};
}

glm::dvec2 Input::GetMouseDelta()
{
    return _mouse_delta;
}

int Input::GetMouseScroll()
{
    return _mouse_scroll;
}

unsigned int Input::GetLastCharInput()
{
    return _last_char_input;
}

//
// GLFW callbacks
//

void Input::KeyInputCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key < 0) return;

    ButtonState &k = _keyboard_state[key];

    if (action == GLFW_PRESS)
    {
        if (!k.held)
            k.pressed = true;
        k.held = true;
    }
    else if (action == GLFW_RELEASE)
    {
        k.held = false;
        k.released = true;
    }
}

void Input::CharInputCallback(GLFWwindow *window, unsigned int codepoint)
{
    _last_char_input = codepoint;
}

void Input::MousePositionCallback(GLFWwindow *window, double x, double y)
{
    _mouse_delta = {x - _mouse_position.x, -(y - _mouse_position.y)};
    _mouse_position = {x, y};
}

void Input::MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    ButtonState &k = _mouse_state[button];

    if (action == GLFW_PRESS)
    {
        if (!k.held)
            k.pressed = true;
        k.held = true;
    }
    else if (action == GLFW_RELEASE)
    {
        k.held = false;
        k.released = true;
    }
}

void Input::MouseScrollCallback(GLFWwindow *window, double delta_x, double delta_y)
{
    _mouse_scroll = (int)delta_y;
}