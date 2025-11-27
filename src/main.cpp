#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ui.h"
#include "shader.h"
#include "soundlib.h"
#include "storage.h"
#include "input.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

void scale_ui(float viewport_width, float viewport_height)
{
    float scale = std::max(viewport_width / 1920.0f, viewport_height / 1080.0f);
    float scaled_virtual_width = 1920.0f * scale;
    float scaled_virtual_height = 1080.0f * scale;

    float offset_x = (viewport_width - scaled_virtual_width)  * 0.5f;
    float offset_y = (viewport_height - scaled_virtual_height) * 0.5f;

    glm::mat4 virtual_to_window = glm::mat4(1.0f);
    virtual_to_window = glm::translate(virtual_to_window, glm::vec3(offset_x, offset_y, 0.0f));
    virtual_to_window = glm::scale(virtual_to_window, glm::vec3(scale, scale, 1.0f));

    glm::mat4 proj = glm::ortho(0.0f, viewport_width, 0.0f, viewport_height, -1.0f, 1.0f);
    glm::mat4 ui_matrix = proj * virtual_to_window;

    ShaderManager::UI_IMAGE_SHADER.Use();
    glUniformMatrix4fv(glGetUniformLocation(ShaderManager::UI_IMAGE_SHADER.GetID(), "ui_matrix"), 1, GL_FALSE, glm::value_ptr(ui_matrix));

    ShaderManager::UI_TEXT_SHADER.Use();
    glUniformMatrix4fv(glGetUniformLocation(ShaderManager::UI_TEXT_SHADER.GetID(), "ui_matrix"), 1, GL_FALSE, glm::value_ptr(ui_matrix));
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #if __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lunacraft", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
        scale_ui(width, height);
    });
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Storage::Init();
    Soundlib::Init();
    ShaderManager::CompileAllShaders();

    scale_ui(WINDOW_WIDTH, WINDOW_HEIGHT);
    UIMainMenu ui_main_menu;

    enum class GameState {MAIN_MENU, IN_GAME};
    static GameState game_state = GameState::MAIN_MENU;

    MouseState mouse_state;

    float delta_time;
    float last_frame_time = 0;
    while (!glfwWindowShouldClose(window))
    {
        float current_time = glfwGetTime();
        delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        //
        // Input
        //

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (!mouse_state.left_clicked && !mouse_state.left_held)
            {
                mouse_state.left_clicked = true;
            }
            else if (mouse_state.left_clicked && !mouse_state.left_held)
            {
                mouse_state.left_clicked = false;
                mouse_state.left_held = true;
            }
        }
        else
        {
            mouse_state.left_clicked = false;
            mouse_state.left_held = false;
        }

        // Cursor position is unbounded (and thus meaningless) when disabled. We don't care about the cursor position in that case anyway.
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);

            GLint viewport_info[4];
            glGetIntegerv(GL_VIEWPORT, viewport_info);
            float viewport_width = viewport_info[2];
            float viewport_height = viewport_info[3];

            float scale = std::max(viewport_width / 1920.0f, viewport_height / 1080.0f);
            float scaled_virtual_width = 1920.0f * scale;
            float scaled_virtual_height = 1080.0f * scale;

            float offset_x = (viewport_width - scaled_virtual_width)  * 0.5f;
            float offset_y = (viewport_height - scaled_virtual_height) * 0.5f;

            // TODO: Make virtual_to_window and window_to_virtual global so we don't have to recompute repeatedly

            glm::mat4 virtual_to_window = glm::mat4(1.0f);
            virtual_to_window = glm::translate(virtual_to_window, glm::vec3(offset_x, offset_y, 0.0f));
            virtual_to_window = glm::scale(virtual_to_window, glm::vec3(scale, scale, 1.0f));

            glm::mat4 window_to_virtual = glm::inverse(virtual_to_window);
            glm::vec4 virtual_mouse = window_to_virtual * glm::vec4(mouse_x, viewport_height - mouse_y, 0.0f, 1.0f);

            mouse_state.position.x = virtual_mouse.x;
            mouse_state.position.y = virtual_mouse.y;
        }

        //
        // Update/Render
        //

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (game_state == GameState::MAIN_MENU)
        {
            ui_main_menu.Update(mouse_state);
            ui_main_menu.Render(delta_time);
        }
        else // IN_GAME
        {

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Soundlib::Exit();
    glfwTerminate();

    return 0;
}
