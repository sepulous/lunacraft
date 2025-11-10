#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "stbi.h"
#include "ui.h"

//
// Screen-space images
//

UIScreenImage::UIScreenImage()
{
    // Vertex array object
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // Vertex buffer object
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // Vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    _position = glm::vec2(0);
    _dimensions = glm::vec2(0);
}

/*
    The position and dimensions use the same coordinate system as texture mapping.
    (x, y) is the bottom left corner of the image quad, and x and y are in [0, 1].

    This coordinate system is easier to think about than normalized device coordinates,
    but still independent of screen size.
*/
UIScreenImage::UIScreenImage(const char *image_path, float x, float y, float width, float height)
{
    // Vertex array object
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // Create texture from image data
    int image_width, image_height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image_data = stbi_load(image_path, &image_width, &image_height, &num_channels, 0);

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    int format = (num_channels == 3) ? GL_RGB : GL_RGBA; // I expect either 3 or 4 channels
    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);

    // Vertex buffer object

    // Convert to NDC
    glm::vec2 bl_ndc = glm::vec2(2*x - 1, 2*y - 1);
    glm::vec2 br_ndc = glm::vec2(2*(x + width) - 1, 2*y - 1);
    glm::vec2 tl_ndc = glm::vec2(2*x - 1, 2*(y + height) - 1);
    glm::vec2 tr_ndc = glm::vec2(2*(x + width) - 1, 2*(y + height) - 1);
    float vertices[] = {
    //  Position----------  UV--------
        bl_ndc.x, bl_ndc.y, 0.0f, 0.0f, 0.0f, // Bottom left
        br_ndc.x, br_ndc.y, 0.0f, 1.0f, 0.0f, // Bottom right
        tr_ndc.x, tr_ndc.y, 0.0f, 1.0f, 1.0f, // Top right
        tr_ndc.x, tr_ndc.y, 0.0f, 1.0f, 1.0f, // Top right
        tl_ndc.x, tl_ndc.y, 0.0f, 0.0f, 1.0f, // Top left
        bl_ndc.x, bl_ndc.y, 0.0f, 0.0f, 0.0f, // Bottom left
    };

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, 5 * 6 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    // Vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    _position = glm::vec2(x, y);
    _dimensions = glm::vec2(width, height);
}

void UIScreenImage::SetImage(const char *image_path)
{
    int image_width, image_height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image_data = stbi_load(image_path, &image_width, &image_height, &num_channels, 0);

    glBindTexture(GL_TEXTURE_2D, _texture); // Already exists since constructor was called
    int format = (num_channels == 3) ? GL_RGB : GL_RGBA; // I expect either 3 or 4 channels
    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);
}

void UIScreenImage::SetPosition(float x, float y)
{
    glm::vec2 bl_ndc = glm::vec2(2*x - 1, 2*y - 1);
    glm::vec2 br_ndc = glm::vec2(2*(x + _dimensions.x) - 1, 2*y - 1);
    glm::vec2 tl_ndc = glm::vec2(2*x - 1, 2*(y + _dimensions.y) - 1);
    glm::vec2 tr_ndc = glm::vec2(2*(x + _dimensions.x) - 1, 2*(y + _dimensions.y) - 1);
    float vertices[] = {
    //  Position----------  UV--------
        bl_ndc.x, bl_ndc.y, 0.0f, 0.0f, 0.0f, // Bottom left
        br_ndc.x, br_ndc.y, 0.0f, 1.0f, 0.0f, // Bottom right
        tr_ndc.x, tr_ndc.y, 0.0f, 1.0f, 1.0f, // Top right
        tr_ndc.x, tr_ndc.y, 0.0f, 1.0f, 1.0f, // Top right
        tl_ndc.x, tl_ndc.y, 0.0f, 0.0f, 1.0f, // Top left
        bl_ndc.x, bl_ndc.y, 0.0f, 0.0f, 0.0f, // Bottom left
    };

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, 5 * 6 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    _position = glm::vec2(x, y);
}

void UIScreenImage::SetDimensions(float width, float height)
{
    glm::vec2 bl_ndc = glm::vec2(2*_position.x - 1, 2*_position.y - 1);
    glm::vec2 br_ndc = glm::vec2(2*(_position.x + width) - 1, 2*_position.y - 1);
    glm::vec2 tl_ndc = glm::vec2(2*_position.x - 1, 2*(_position.y + height) - 1);
    glm::vec2 tr_ndc = glm::vec2(2*(_position.x + width) - 1, 2*(_position.y + height) - 1);
    float vertices[] = {
    //  Position----------  UV--------
        bl_ndc.x, bl_ndc.y, 0.0f, 0.0f, 0.0f, // Bottom left
        br_ndc.x, br_ndc.y, 0.0f, 1.0f, 0.0f, // Bottom right
        tr_ndc.x, tr_ndc.y, 0.0f, 1.0f, 1.0f, // Top right
        tr_ndc.x, tr_ndc.y, 0.0f, 1.0f, 1.0f, // Top right
        tl_ndc.x, tl_ndc.y, 0.0f, 0.0f, 1.0f, // Top left
        bl_ndc.x, bl_ndc.y, 0.0f, 0.0f, 0.0f, // Bottom left
    };

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, 5 * 6 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    _dimensions = glm::vec2(width, height);
}

void UIScreenImage::Render()
{
    glBindVertexArray(_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

//
// Main Menu
//

UIMainMenu::UIMainMenu()
{
    _lunacraft_text = new UIScreenImage("../assets/images/lunacraft.png", 0.02f, 0.88f, 0.35f, 0.1f);

    std::vector<std::string> background_image_paths = {
        "../assets/images/main_menu_1.png",
        "../assets/images/main_menu_2.png",
        "../assets/images/main_menu_3.png",
        "../assets/images/main_menu_4.png",
        "../assets/images/main_menu_5.png"
    };

    _background_images = new UIScreenImage[background_image_paths.size()];
    for (int i = 0; i < background_image_paths.size(); i++)
        _background_images[i] = UIScreenImage(background_image_paths[i].c_str(), 0, 0, 1, 1);

    // // _buttons[0] = UIButton(...);
    // // ...
    // // _buttons[9] = UIButton(...);
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
    // Background images
    //
    
    Shader screen_image_shader = ShaderManager::GetShader(SHADER_SCREEN_IMAGE);

    // Fully visible for 8 seconds, fades out in 1 second
    const float opaque_time = 8.0f;
    const float fade_time = 1.0f;
    const float scale_speed = 0.05f;

    if (_current_bg_time < opaque_time) // Current background is fully opaque
    {
        float scale = scale_speed * _current_bg_time + 1.0f;
        screen_image_shader.SetFloat("scale", scale);
        screen_image_shader.SetFloat("opacity", 1.0f);
        _background_images[_current_bg].Render();
    }
    else // Cross-fading
    {
        float scale, opacity;

        // Current
        scale = scale_speed * _current_bg_time + 1.0f;
        opacity = (opaque_time + fade_time) - _current_bg_time;
        screen_image_shader.SetFloat("scale", scale);
        screen_image_shader.SetFloat("opacity", opacity);
        _background_images[_current_bg].Render();

        // Next
        scale = scale_speed * (_current_bg_time - opaque_time) + 1.0f;
        opacity = (_current_bg_time - opaque_time) / fade_time;
        screen_image_shader.SetFloat("scale", scale);
        screen_image_shader.SetFloat("opacity", opacity);
        _background_images[(_current_bg + 1) % 5].Render();

        if (_current_bg_time >= opaque_time + fade_time)
        {
            _current_bg_time = fade_time;
            _current_bg = (_current_bg + 1) % 5;
        }
    }

    _current_bg_time += delta_time;

    //
    // Lunacraft text
    //

    screen_image_shader.Use();
    screen_image_shader.SetFloat("scale", 1.0f);
    screen_image_shader.SetFloat("opacity", 1.0f);
    _lunacraft_text->Render();
    


    // for (UIButton button : _buttons)
    //     button.Render();

    // if (_moon_settings_menu.IsActive())
    //     _moon_settings_menu.Render();

    // if (_options_menu.IsActive())
    //     _options_menu.Render();
}
