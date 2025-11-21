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

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
// int current_window_width = WINDOW_WIDTH;
// int current_window_height = WINDOW_HEIGHT;
glm::vec2 current_viewport = glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);
glm::vec2 new_viewport = glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);

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
        // current_window_width = width;
        // current_window_height = height;
        new_viewport = glm::vec2(width, height);
    });
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Storage::Init();
    Soundlib::Init();
    ShaderManager::CompileAllShaders();

    UIMainMenu ui_main_menu;
    // UIPauseMenu ui_pause_menu;

    enum class GameState {MAIN_MENU, IN_GAME};
    static GameState game_state = GameState::MAIN_MENU;

    UIScreenText sample_text("Sample text", 1.0f, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f});

    float delta_time;
    float last_frame_time = 0;
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float current_time = glfwGetTime();
        delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (game_state == GameState::MAIN_MENU)
        {
            if (current_viewport != new_viewport)
            {
                ui_main_menu.Rescale(current_viewport, new_viewport);
                current_viewport = new_viewport;
            }
            ui_main_menu.Update();
            ui_main_menu.Render(delta_time);

            sample_text.Render();

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
