#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stbi.h"
#include "ui.h"

//
// Main Menu
//

UIMainMenu::UIMainMenu()
{
    _bg_shader.MakeProgram("../shaders/main_menu_bg.vert", "../shaders/main_menu_bg.frag");

    //
    // Generate background textures
    //

    std::vector<std::string> background_image_paths = {
        "../assets/images/main_menu_1.png",
        "../assets/images/main_menu_2.png",
        "../assets/images/main_menu_3.png",
        "../assets/images/main_menu_4.png",
        "../assets/images/main_menu_5.png"
    };

    for (int i = 0; i < background_image_paths.size(); i++)
    {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *bg_image_data = stbi_load(background_image_paths[i].c_str(), &width, &height, &nrChannels, 0);

        glGenTextures(1, &_bg_textures[i]);
        glBindTexture(GL_TEXTURE_2D, _bg_textures[i]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        int format = (nrChannels == 3) ? GL_RGB : GL_RGBA; // I expect either 3 or 4 channels
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, bg_image_data);

        stbi_image_free(bg_image_data);
    }

    //
    // Generate background quad
    //

    float bg_vertices[] = {
    //  Position----------  UV--------
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Bottom right triangle
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, //
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f, //
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f, // Top left triangle
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f, //
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f  //
    };

    glGenVertexArrays(1, &_bg_vao);
    glBindVertexArray(_bg_vao);

    unsigned int bg_vbo;
    glGenBuffers(1, &bg_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, bg_vbo);
    glBufferData(GL_ARRAY_BUFFER, 5 * 6 * sizeof(float), bg_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // _buttons[0] = UIButton(...);
    // ...
    // _buttons[9] = UIButton(...);
}

void UIMainMenu::Update()
{
    // for (UIButton button : _buttons)
    //     button.Update();

    // if (_moon_settings_menu.IsActive())
    //     _moon_settings_menu.Update();

    // if (_options_menu.IsActive())
    //     _options_menu.Update();
}

void UIMainMenu::Render(float delta_time)
{
    //
    // Render background images
    //
    
    _bg_shader.Use();

    // Fully visible for 8 seconds, fades out in 1 second
    const float opaque_time = 8.0f;
    const float fade_time = 1.0f;
    const float scale_speed = 0.05f;

    if (_current_bg_time < opaque_time) // Current background is fully opaque
    {
        float scale = scale_speed * _current_bg_time + 1.0f;
        _bg_shader.SetFloat("scale", scale);
        _bg_shader.SetFloat("opacity", 1.0f);

        glBindVertexArray(_bg_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _bg_textures[_current_bg]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    else // Cross-fading
    {
        float scale, opacity;

        // Current
        scale = scale_speed * _current_bg_time + 1.0f;
        opacity = (opaque_time + fade_time) - _current_bg_time;
        _bg_shader.SetFloat("scale", scale);
        _bg_shader.SetFloat("opacity", opacity);

        glBindVertexArray(_bg_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _bg_textures[_current_bg]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Next
        scale = scale_speed * (_current_bg_time - opaque_time) + 1.0f;
        opacity = (_current_bg_time - opaque_time) / fade_time;
        _bg_shader.SetFloat("scale", scale);
        _bg_shader.SetFloat("opacity", opacity);

        glBindVertexArray(_bg_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _bg_textures[(_current_bg + 1) % 5]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        if (_current_bg_time >= opaque_time + fade_time)
        {
            _current_bg_time = fade_time;
            _current_bg = (_current_bg + 1) % 5;
        }
    }

    _current_bg_time += delta_time;
    


    // for (UIButton button : _buttons)
    //     button.Render();

    // if (_moon_settings_menu.IsActive())
    //     _moon_settings_menu.Render();

    // if (_options_menu.IsActive())
    //     _options_menu.Render();
}
