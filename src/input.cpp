
#include "input.h"
#include "viewport.h"

//
// State
//

ButtonState  Input::keyboard_state_[GLFW_KEY_LAST + 1];
ButtonState  Input::mouse_state_[GLFW_MOUSE_BUTTON_LAST + 1];
glm::dvec2   Input::mouse_position_;
glm::dvec2   Input::mouse_delta_;
int          Input::mouse_scroll_;
unsigned int Input::last_char_input_;

//
// API
//

void Input::BeginFrame()
{
    for (auto &k : keyboard_state_)
    {
        k.pressed  = false;
        k.released = false;
    }

    for (auto &k : mouse_state_)
    {
        k.pressed  = false;
        k.released = false;
    }

    last_char_input_ = 0;
    mouse_scroll_ = 0;
    mouse_delta_ = glm::dvec2(0);
}

bool Input::IsKeyHeld(int key)
{
    return keyboard_state_[key].held;
}

bool Input::IsKeyPressed(int key)
{
    return keyboard_state_[key].pressed;
}

bool Input::IsKeyReleased(int key)
{
    return keyboard_state_[key].released;
}

bool Input::IsMouseButtonHeld(int button)
{
    return mouse_state_[button].held;
}

bool Input::IsMouseButtonPressed(int button)
{
    return mouse_state_[button].pressed;
}

bool Input::IsMouseButtonReleased(int button)
{
    return mouse_state_[button].released;
}

glm::dvec2 Input::GetMousePosition()
{
    return mouse_position_;
}

void Input::SetMousePosition(double x, double y)
{
    mouse_position_ = {x, y};
}

glm::dvec2 Input::GetVirtualMousePosition(const glm::mat4 &virtual_to_window)
{
    glm::vec4 virtual_mouse = glm::inverse(virtual_to_window) * glm::vec4(mouse_position_.x, Viewport::GetDimensions().y - mouse_position_.y, 0.0f, 1.0f);
    return {virtual_mouse.x, virtual_mouse.y};
}

glm::dvec2 Input::GetMouseDelta()
{
    return mouse_delta_;
}

int Input::GetMouseScroll()
{
    return mouse_scroll_;
}

unsigned int Input::GetLastCharInput()
{
    return last_char_input_;
}

//
// GLFW callbacks
//

void Input::KeyInputCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key < 0) return;

    ButtonState &k = keyboard_state_[key];

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
    last_char_input_ = codepoint;
}

void Input::MousePositionCallback(GLFWwindow *window, double x, double y)
{
    mouse_delta_ = {x - mouse_position_.x, -(y - mouse_position_.y)};
    mouse_position_ = {x, y};
}

void Input::MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    ButtonState &k = mouse_state_[button];

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
    mouse_scroll_ = (int)delta_y;
}
