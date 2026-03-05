#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "viewport.h"

struct ButtonState
{
    bool held = false; // Currently held
    bool pressed = false; // Went down this frame
    bool released = false; // Went up this frame
};

class Input
{
// API
public:
    static void BeginFrame();

    static bool IsKeyHeld(int key);
    static bool IsKeyPressed(int key);
    static bool IsKeyReleased(int key);

    static bool IsMouseButtonHeld(int button);
    static bool IsMouseButtonPressed(int button);
    static bool IsMouseButtonReleased(int button);

    static glm::dvec2 GetMousePosition();
    static void SetMousePosition(double x, double y);
    static glm::dvec2 GetVirtualMousePosition(const glm::mat4 &virtual_to_window);
    static glm::dvec2 GetMouseDelta();
    static int GetMouseScroll();

    static unsigned int GetLastCharInput();

// GLFW callbacks
public:
    static void KeyInputCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void CharInputCallback(GLFWwindow *window, unsigned int codepoint);
    static void MousePositionCallback(GLFWwindow *window, double x, double y);
    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void MouseScrollCallback(GLFWwindow *window, double delta_x, double delta_y);

// State
private:
    static ButtonState keyboard_state_[GLFW_KEY_LAST + 1];
    static ButtonState mouse_state_[GLFW_MOUSE_BUTTON_LAST + 1];
    static glm::dvec2 mouse_position_;
    static glm::dvec2 mouse_delta_;
    static int mouse_scroll_;
    static unsigned int last_char_input_;
};
