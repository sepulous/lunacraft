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
#include "options.h"

glm::vec2 viewport = {1280, 720};
glm::mat4 ui_window_to_virtual = glm::mat4(1.0f);
glm::mat4 ui_virtual_to_window = glm::mat4(1.0f);

// NOTE: Must compile shaders before calling this!
void ui_rescale()
{
    glm::mat4 proj = glm::ortho(0.0f, viewport.x, 0.0f, viewport.y, -1.0f, 1.0f);
    glm::mat4 ui_matrix = proj * ui_virtual_to_window;

    ShaderManager::UI_IMAGE_SHADER.Use();
    glUniformMatrix4fv(glGetUniformLocation(ShaderManager::UI_IMAGE_SHADER.GetID(), "ui_matrix"), 1, GL_FALSE, glm::value_ptr(ui_matrix));

    ShaderManager::UI_TEXT_SHADER.Use();
    glUniformMatrix4fv(glGetUniformLocation(ShaderManager::UI_TEXT_SHADER.GetID(), "ui_matrix"), 1, GL_FALSE, glm::value_ptr(ui_matrix));
}

void ui_update_transforms()
{
    float scale = std::max(viewport.x / VIRTUAL_UI_WIDTH, viewport.y / VIRTUAL_UI_HEIGHT);
    float scaled_virtual_width = VIRTUAL_UI_WIDTH * scale;
    float scaled_virtual_height = VIRTUAL_UI_HEIGHT * scale;
    float offset_x = (viewport.x - scaled_virtual_width)  * 0.5f;
    float offset_y = (viewport.y - scaled_virtual_height) * 0.5f;
    ui_virtual_to_window = glm::mat4(1.0f);
    ui_virtual_to_window = glm::translate(ui_virtual_to_window, glm::vec3(offset_x, offset_y, 0.0f));
    ui_virtual_to_window = glm::scale(ui_virtual_to_window, glm::vec3(scale, scale, 1.0f));
    ui_window_to_virtual = glm::inverse(ui_virtual_to_window);
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

    GLFWwindow *window = glfwCreateWindow(viewport.x, viewport.y, "Lunacraft", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
        viewport = {width, height};
        ui_update_transforms();
        ui_rescale();
    });
    glViewport(0, 0, viewport.x, viewport.y);
    ui_update_transforms();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Storage::Init();
    OptionsManager::Init();
    ShaderManager::CompileAllShaders();
    Soundlib::Init();

    ui_rescale();
    UIMainMenu ui_main_menu(window);

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

            glm::vec4 virtual_mouse = ui_window_to_virtual * glm::vec4(mouse_x, viewport.y - mouse_y, 0.0f, 1.0f);
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
            ui_main_menu.Update(delta_time, mouse_state);
            ui_main_menu.Render(delta_time);
        }
        else // IN_GAME
        {

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    OptionsManager::SaveOptions();
    Soundlib::Exit();
    glfwTerminate();

    return 0;
}
