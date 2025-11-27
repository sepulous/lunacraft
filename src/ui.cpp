#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ui.h"
#include "storage.h"
#include "input.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image/stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb_truetype/stb_truetype.h>

//
// Main Menu
//

UIMainMenu::UIMainMenu()
{
    // Lunacraft logo
    _lunacraft_logo.LoadImage(Storage::ASSET_DIR / "images" / "lunacraft.png");
    _lunacraft_logo.SetPosition({70, 875});
    _lunacraft_logo.SetSize({859, 130});

    // Background images
    int i = 1;
    for (UIImage& background_image : _background_images)
    {
        std::string image_path = std::string("main_menu_") + std::to_string(i) + ".png";
        background_image.LoadImage(Storage::ASSET_DIR / "images" / image_path);
        background_image.SetPosition({0, 0});
        background_image.SetSize({VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});
        i++;
    }

    // Moon buttons
    i = 0;
    for (UIButton& moon_button : _moon_buttons)
    {
        glm::vec2 button_position = {595, 705 - i*(105 + 25)};
        glm::vec2 button_size = {820, 105};
        moon_button.SetSize(button_size);
        moon_button.SetPosition(button_position);
        moon_button.SetClickAction([this]() { _moon_settings_menu.SetActive(true); });

        std::string button_text = std::string("Moon ") + "ABCD"[i] + " - Unexplored";
        glm::vec2 text_size = UIText::GetTextSizeInPixels(button_text, 0.5f);
        moon_button.GetText().SetPosition({
            button_position.x + (button_size.x / 2.0f) - (text_size.x / 2.0f),
            button_position.y + (button_size.y / 2.0f) - (text_size.y / 2.0f)
        });
        moon_button.SetText(button_text, 0.5f, {0.0f, 0.0f, 0.0f, 1.0f});

        i++;
    }

    // Reset buttons
    i = 0;
    for (UIButton& reset_button : _reset_buttons)
    {
        glm::vec2 button_position = {1475, 715 - i*(80 + 50)};
        glm::vec2 button_size = {210, 80};
        reset_button.SetSize(button_size);
        reset_button.SetPosition(button_position);

        glm::vec2 text_size = UIText::GetTextSizeInPixels("Reset", 0.425f);
        reset_button.GetText().SetPosition({
            button_position.x + (button_size.x / 2.0f) - (text_size.x / 2.0f),
            button_position.y + (button_size.y / 2.0f) - (text_size.y / 2.0f)
        });
        reset_button.SetText("Reset", 0.425f, {0.9f, 0.0f, 0.0f, 1.0f});

        i++;
    }

    // Options button
    glm::vec2 options_button_position = {785, 175};
    glm::vec2 options_button_size = {390, 105};
    float options_font_size = 0.6f;
    _options_button.SetSize(options_button_size);
    _options_button.SetPosition(options_button_position);
    glm::vec2 options_text_size = UIText::GetTextSizeInPixels("Options", options_font_size);
    _options_button.GetText().SetPosition({
        options_button_position.x + (options_button_size.x / 2.0f) - (options_text_size.x / 2.0f),
        options_button_position.y + (options_button_size.y / 2.0f) - (options_text_size.y / 2.0f)
    });
    _options_button.SetText("Options", options_font_size, {0.0f, 0.0f, 0.0f, 1.0f});

    // Quit button
    glm::vec2 quit_button_position = {1195, 175};
    glm::vec2 quit_button_size = {220, 105};
    float quit_font_size = 0.6f;
    _quit_button.SetSize(quit_button_size);
    _quit_button.SetPosition(quit_button_position);
    glm::vec2 quit_text_size = UIText::GetTextSizeInPixels("Quit", quit_font_size);
    _quit_button.GetText().SetPosition({
        quit_button_position.x + (quit_button_size.x / 2.0f) - (quit_text_size.x / 2.0f),
        quit_button_position.y + (quit_button_size.y / 2.0f) - (quit_text_size.y / 2.0f)
    });
    _quit_button.SetText("Quit", quit_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
}

void UIMainMenu::Update(MouseState mouse_state)
{
    if (_moon_settings_menu.IsActive())
    {
        _moon_settings_menu.Update(mouse_state);
    }
    else // Player can't interact with buttons behind active menus
    {
        for (UIButton& moon_button : _moon_buttons)
            moon_button.Update(mouse_state);

        for (UIButton& reset_button : _reset_buttons)
            reset_button.Update(mouse_state);

        _options_button.Update(mouse_state);
        _quit_button.Update(mouse_state);
    }
}

void UIMainMenu::Render(float delta_time)
{
    Shader ui_image_shader = ShaderManager::UI_IMAGE_SHADER;
    
    //
    // Background images
    //

    ui_image_shader.Use();
    ui_image_shader.SetFloat("darkness", 0.0f);

    // Fully visible for 8 seconds, fades out in 1 second
    const float opaque_time = 8.0f;
    const float fade_time = 1.0f;
    const float scale_speed = 0.04f;
    glm::vec2 current_position;
    glm::vec2 offset;

    if (_current_background_time < opaque_time) // Current background is fully opaque
    {
        float scale = scale_speed * _current_background_time + 1.0f;
        ui_image_shader.SetFloat("opacity", 1.0f);

        current_position = _background_images[_current_background].GetPosition();
        offset = {(VIRTUAL_UI_WIDTH * (scale - 1.0f)) / 2.0f, (VIRTUAL_UI_HEIGHT * (scale - 1.0f)) / 2.0f};
        _background_images[_current_background].SetSize(scale * glm::vec2(VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT));
        _background_images[_current_background].SetPosition({-offset.x, -offset.y});
        _background_images[_current_background].Render();
    }
    else // Cross-fading
    {
        float scale, opacity;

        // Current
        scale = scale_speed * _current_background_time + 1.0f;
        opacity = (opaque_time + fade_time) - _current_background_time;
        ui_image_shader.SetFloat("opacity", opacity);

        current_position = _background_images[_current_background].GetPosition();
        offset = {(VIRTUAL_UI_WIDTH * (scale - 1.0f)) / 2.0f, (VIRTUAL_UI_HEIGHT * (scale - 1.0f)) / 2.0f};
        _background_images[_current_background].SetSize(scale * glm::vec2(VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT));
        _background_images[_current_background].SetPosition({-offset.x, -offset.y});
        _background_images[_current_background].Render();

        // Next
        scale = scale_speed * (_current_background_time - opaque_time) + 1.0f;
        opacity = (_current_background_time - opaque_time) / fade_time;
        ui_image_shader.SetFloat("opacity", opacity);

        current_position = _background_images[(_current_background + 1) % 5].GetPosition();
        offset = {(VIRTUAL_UI_WIDTH * (scale - 1.0f)) / 2.0f, (VIRTUAL_UI_HEIGHT * (scale - 1.0f)) / 2.0f};
        _background_images[(_current_background + 1) % 5].SetSize(scale * glm::vec2(VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT));
        _background_images[(_current_background + 1) % 5].SetPosition({-offset.x, -offset.y});
        _background_images[(_current_background + 1) % 5].Render();

        if (_current_background_time >= opaque_time + fade_time)
        {
            _current_background_time = fade_time;
            _background_images[_current_background].SetSize({VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});
            _background_images[_current_background].SetPosition({0, 0});
            _current_background = (_current_background + 1) % 5;
        }
    }

    _current_background_time += delta_time;


    ui_image_shader.SetFloat("opacity", 1.0f);

    _lunacraft_logo.Render();

    for (UIButton& moon_button : _moon_buttons)
        moon_button.Render();

    for (UIButton& reset_button : _reset_buttons)
        reset_button.Render();

    _options_button.Render();
    _quit_button.Render();

    ui_image_shader.SetFloat("darkness", 0.0f);
    if (_moon_settings_menu.IsActive())
        _moon_settings_menu.Render();
}

//
// Moon Settings Menu
//

UIMoonSettingsMenu::UIMoonSettingsMenu()
{
    _background.LoadImage(Storage::ASSET_DIR / "images" / "ui_menu_bg.png", GL_NEAREST);
    float bg_width = 1200;
    float bg_height = 700;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    _background.SetSize({bg_width, bg_height});
    _background.SetPosition({bg_pos_x, bg_pos_y});

    glm::vec2 back_button_position = {bg_pos_x + 50, bg_pos_y + 40};
    glm::vec2 back_button_size = {160, 80};
    float back_font_size = 0.4f;
    _back_button.SetPosition(back_button_position);
    _back_button.SetSize(back_button_size);
    _back_button.SetText("Back", back_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 back_text_size = UIText::GetTextSizeInPixels("Back", back_font_size);
    _back_button.GetText().SetPosition({
        back_button_position.x + (back_button_size.x / 2.0f) - (back_text_size.x / 2.0f),
        back_button_position.y + (back_button_size.y / 2.0f) - (back_text_size.y / 2.0f)
    });
    _back_button.SetClickAction([this]() { _active = false; });
}

void UIMoonSettingsMenu::SetActive(bool status)
{
    _active = status;
}

bool UIMoonSettingsMenu::IsActive()
{
    return _active;
}

void UIMoonSettingsMenu::Update(MouseState mouse_state)
{
    _back_button.Update(mouse_state);
}

void UIMoonSettingsMenu::Render()
{
    _background.Render();
    _back_button.Render();
}

//
// Images
//

UIImage::UIImage(GLint filtering)
{
    // Vertex array object
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // Vertex buffer object
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // Vertex attribute pointer
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    _position = glm::vec2(0);
    _size = glm::vec2(0);
}

UIImage::UIImage(std::filesystem::path image_path, glm::vec2 position, glm::vec2 size, GLint filtering)
{
    // Vertex array object
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // Create texture from image data
    int image_width, image_height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image_data = stbi_load(image_path.c_str(), &image_width, &image_height, &num_channels, 0);

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    int format = (num_channels == 3) ? GL_RGB : GL_RGBA; // I expect either 3 or 4 channels
    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);

    float vertices[] = {
    //  Position--------------------------------  UV--------
        position.x,          position.y,          0.0f, 0.0f, // Bottom left
        position.x + size.x, position.y,          1.0f, 0.0f, // Bottom right
        position.x + size.x, position.y + size.y, 1.0f, 1.0f, // Top right
        position.x + size.x, position.y + size.y, 1.0f, 1.0f, // Top right
        position.x,          position.y + size.y, 0.0f, 1.0f, // Top left
        position.x,          position.y,          0.0f, 0.0f, // Bottom left
    };

    // Vertex buffer object
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    // Vertex attribute pointer
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    _position = position;
    _size = size;
}

void UIImage::LoadImage(std::filesystem::path image_path, GLint filtering)
{
    int image_width, image_height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image_data = stbi_load(image_path.c_str(), &image_width, &image_height, &num_channels, 0);

    glBindTexture(GL_TEXTURE_2D, _texture); // Already exists since constructor was called
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    int format = (num_channels == 3) ? GL_RGB : GL_RGBA; // I expect either 3 or 4 channels
    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);
}

void UIImage::SetPosition(glm::vec2 position)
{
    float vertices[] = {
    //  Position----------------------------------  UV--------
        position.x,           position.y,           0.0f, 0.0f, // Bottom left
        position.x + _size.x, position.y,           1.0f, 0.0f, // Bottom right
        position.x + _size.x, position.y + _size.y, 1.0f, 1.0f, // Top right
        position.x + _size.x, position.y + _size.y, 1.0f, 1.0f, // Top right
        position.x,           position.y + _size.y, 0.0f, 1.0f, // Top left
        position.x,           position.y,           0.0f, 0.0f, // Bottom left
    };
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    _position = position;
}

glm::vec2 UIImage::GetPosition()
{
    return _position;
}

void UIImage::SetSize(glm::vec2 size)
{
    float vertices[] = {
    //  Position----------------------------------  UV--------
        _position.x,          _position.y,          0.0f, 0.0f, // Bottom left
        _position.x + size.x, _position.y,          1.0f, 0.0f, // Bottom right
        _position.x + size.x, _position.y + size.y, 1.0f, 1.0f, // Top right
        _position.x + size.x, _position.y + size.y, 1.0f, 1.0f, // Top right
        _position.x,          _position.y + size.y, 0.0f, 1.0f, // Top left
        _position.x,          _position.y,          0.0f, 0.0f, // Bottom left
    };

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    _size = size;
}

glm::vec2 UIImage::GetSize()
{
    return _size;
}

void UIImage::Render()
{
    ShaderManager::UI_IMAGE_SHADER.Use();
    glBindVertexArray(_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

//
// Text
//

GLuint UIText::_atlas_texture = 0;
stbtt_packedchar UIText::_packed_chars[95];
stbtt_aligned_quad UIText::_aligned_quads[95];

void UIText::_SetupFont()
{
    std::ifstream inputStream(Storage::ASSET_DIR / "fonts" / "ack.ttf", std::ios::binary);

    inputStream.seekg(0, std::ios::end);
    auto&& fontFileSize = inputStream.tellg();
    inputStream.seekg(0, std::ios::beg);

    uint8_t* fontDataBuf = new uint8_t[fontFileSize];

    inputStream.read((char*)fontDataBuf, fontFileSize);

    stbtt_fontinfo fontInfo = {};
    if(!stbtt_InitFont(&fontInfo, fontDataBuf, 0))
        std::cerr << "stbtt_InitFont() Failed!\n";

    const size_t font_atlas_width = 1024;
    const size_t font_atlas_height = 1024;
    unsigned char *font_atlas_texture_data = new uint8_t[font_atlas_width * font_atlas_height];
    
    stbtt_pack_context ctx;
    stbtt_PackBegin(
        &ctx,
        font_atlas_texture_data,
        font_atlas_width,
        font_atlas_height,
        0,                        // Stride in bytes
        1,                        // Padding between the glyphs
        nullptr
    );
    stbtt_PackFontRange(
        &ctx,                         // stbtt_pack_context
        fontDataBuf,                  // Font Atlas texture data
        0,                            // Font Index                                 
        128,                           // Size of font in pixels. (Use STBTT_POINT_SIZE(fontSize) to use points) 
        (int)' ',                     // Code point of the first charecter
        95,                           // No. of charecters to be included in the font atlas 
        _packed_chars                 // stbtt_packedchar array, this struct will contain the data to render a glyph
    );
    stbtt_PackEnd(&ctx);

    for (int i = 0; i < 95; i++)
    {
        float unused_x, unused_y;

        stbtt_GetPackedQuad(
            _packed_chars,              // Array of stbtt_packedchar
            font_atlas_width,                      // Width of the font atlas texture
            font_atlas_height,                     // Height of the font atlas texture
            i,                                   // Index of the glyph
            &unused_x, &unused_y,                  // current position of the glyph in screen pixel coordinates, (not required as we have a different corrdinate system)
            &_aligned_quads[i],         // stbtt_alligned_quad struct. (this struct mainly consists of the texture coordinates)
            0                                    // Allign X and Y position to a integer (doesn't matter because we are not using 'unusedX' and 'unusedY')
        );
    }

    delete[] fontDataBuf;
    
    // Set up font texture
    glGenTextures(1, &_atlas_texture);
    glBindTexture(GL_TEXTURE_2D, _atlas_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, font_atlas_width, font_atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, font_atlas_texture_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] font_atlas_texture_data;
}

UIText::UIText()
{
    if (_atlas_texture == 0) // Font must be initialized
    {
        UIText::_SetupFont();
    }

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Position and UV coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float))); // Color
    glEnableVertexAttribArray(1);

    _text = "";
    _font_size = 1.0f;
    _position = glm::vec2(0.0f, 0.0f);
    _color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

UIText::UIText(std::string text, float font_size, glm::vec2 position, glm::vec4 color)
{
    if (_atlas_texture == 0) // Font must be initialized
    {
        UIText::_SetupFont();
    }

    _text = text;
    _font_size = font_size;
    _position = position;
    _color = color;

    int order[6] = { 0, 1, 2, 0, 2, 3 };
    glm::vec2 localPosition = position;
    float vertices[text.length() * 6 * (4 + 4)];
    int vertices_index = 0;
    for (char ch : text)
    {
        // Check if the charecter glyph is in the font atlas.
        if (ch >= (int)' ' && ch <= (int)' ' + 95)
        {
            // Retrive the data that is used to render a glyph of charecter 'ch'
            stbtt_packedchar* packedChar = &_packed_chars[ch - (int)' ']; 
            stbtt_aligned_quad* alignedQuad = &_aligned_quads[ch - (int)' '];

            // The units of the fields of the above structs are in pixels, 
            // convert them to a unit of what we want be multilplying to pixelScale  
            glm::vec2 glyphSize = 
            {
                (packedChar->x1 - packedChar->x0) * font_size,
                (packedChar->y1 - packedChar->y0) * font_size
            };

            glm::vec2 glyphBoundingBoxBottomLeft = 
            {
                localPosition.x + (packedChar->xoff * font_size),
                localPosition.y - (packedChar->yoff + packedChar->y1 - packedChar->y0) * font_size
            };

            // The order of vertices of a quad goes top-right, top-left, bottom-left, bottom-right
            glm::vec2 glyphVertices[4] = 
            {
                { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
                { glyphBoundingBoxBottomLeft.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
                { glyphBoundingBoxBottomLeft.x, glyphBoundingBoxBottomLeft.y },
                { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y },
            };

            glm::vec2 glyphTextureCoords[4] = 
            {
                { alignedQuad->s1, alignedQuad->t0 },
                { alignedQuad->s0, alignedQuad->t0 },
                { alignedQuad->s0, alignedQuad->t1 },
                { alignedQuad->s1, alignedQuad->t1 },
            };

            for (int i = 0; i < 6; i++)
            {
                vertices[vertices_index] = glyphVertices[order[i]].x;
                vertices[vertices_index + 1] = glyphVertices[order[i]].y;
                vertices[vertices_index + 2] = glyphTextureCoords[order[i]].x;
                vertices[vertices_index + 3] = glyphTextureCoords[order[i]].y;
                vertices[vertices_index + 4] = color.r;
                vertices[vertices_index + 5] = color.g;
                vertices[vertices_index + 6] = color.b;
                vertices[vertices_index + 7] = color.a;
                vertices_index += 8;
            }

            localPosition.x += packedChar->xadvance * font_size;
        }
        else if (ch == '\n')
        {
            localPosition.y -= 128 * font_size;
            localPosition.x = position.x;
        }
    }

    // Set up VAO and VBO
    size_t vbo_size = text.length() * 6 * (4 + 4) * sizeof(float); // (# chars) x (6 verts/char) x (3 + 4 + 2 floats/vert) x sizeof(float)
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_size, vertices, GL_DYNAMIC_DRAW);
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Position and UV coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float))); // Color
    glEnableVertexAttribArray(1);
}

void UIText::Render()
{
    Shader text_shader = ShaderManager::UI_TEXT_SHADER;
    text_shader.Use();
    glBindTexture(GL_TEXTURE_2D, _atlas_texture);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(text_shader.GetID(), "uFontAtlasTexture"), 0);
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, _text.length() * 6 * (4 + 4));
}

void UIText::SetText(std::string text)
{
    int order[6] = { 0, 1, 2, 0, 2, 3 };
    glm::vec2 localPosition = _position;
    float vertices[text.length() * 6 * (4 + 4)];
    int vertices_index = 0;
    for (char ch : text)
    {
        // Check if the charecter glyph is in the font atlas.
        if (ch >= (int)' ' && ch <= (int)' ' + 95)
        {
            // Retrive the data that is used to render a glyph of charecter 'ch'
            stbtt_packedchar* packedChar = &_packed_chars[ch - (int)' ']; 
            stbtt_aligned_quad* alignedQuad = &_aligned_quads[ch - (int)' '];

            // The units of the fields of the above structs are in pixels, 
            // convert them to a unit of what we want be multilplying to pixelScale  
            glm::vec2 glyphSize = 
            {
                (packedChar->x1 - packedChar->x0) * _font_size,
                (packedChar->y1 - packedChar->y0) * _font_size
            };

            glm::vec2 glyphBoundingBoxBottomLeft = 
            {
                localPosition.x + (packedChar->xoff * _font_size),
                localPosition.y - (packedChar->yoff + packedChar->y1 - packedChar->y0) * _font_size
            };

            // The order of vertices of a quad goes top-right, top-left, bottom-left, bottom-right
            glm::vec2 glyphVertices[4] = 
            {
                { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
                { glyphBoundingBoxBottomLeft.x,               glyphBoundingBoxBottomLeft.y + glyphSize.y },
                { glyphBoundingBoxBottomLeft.x,               glyphBoundingBoxBottomLeft.y },
                { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y },
            };

            glm::vec2 glyphTextureCoords[4] = 
            {
                { alignedQuad->s1, alignedQuad->t0 },
                { alignedQuad->s0, alignedQuad->t0 },
                { alignedQuad->s0, alignedQuad->t1 },
                { alignedQuad->s1, alignedQuad->t1 },
            };

            for (int i = 0; i < 6; i++)
            {
                vertices[vertices_index] = glyphVertices[order[i]].x;
                vertices[vertices_index + 1] = glyphVertices[order[i]].y;
                vertices[vertices_index + 2] = glyphTextureCoords[order[i]].x;
                vertices[vertices_index + 3] = glyphTextureCoords[order[i]].y;
                vertices[vertices_index + 4] = _color.r;
                vertices[vertices_index + 5] = _color.g;
                vertices[vertices_index + 6] = _color.b;
                vertices[vertices_index + 7] = _color.a;
                vertices_index += 8;
            }

            localPosition.x += packedChar->xadvance * _font_size;
        }
        else if (ch == '\n')
        {
            localPosition.y -= 128 * _font_size;
            localPosition.x = _position.x;
        }
    }

    size_t vbo_size = text.length() * 6 * (4 + 4) * sizeof(float); // (# chars) x (6 verts/char) x (3 + 4 + 2 floats/vert) x sizeof(float)
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_size, vertices, GL_DYNAMIC_DRAW);

    _text = text;
}

void UIText::SetPosition(glm::vec2 position)
{
    _position = position;
    SetText(_text);
}

void UIText::SetFontSize(float font_size)
{
    _font_size = font_size;
    SetText(_text);
}

void UIText::SetColor(glm::vec4 color)
{
    _color = color;
    SetText(_text);
}

glm::vec2 UIText::GetTextSizeInPixels(std::string text, float font_size)
{
    float width, height = 0;
    float current_line_width = 0;
    for (int i = 0; i < text.length(); i++)
    {
        char ch = text.at(i);
        if (ch >= (int)' ' && ch <= (int)' ' + 95)
        {
            stbtt_packedchar* packedChar = &_packed_chars[ch - (int)' '];
            current_line_width += packedChar->xadvance * font_size;
        }
        
        if (ch == '\n' || i == text.length() - 1)
        {
            if (current_line_width > width)
                width = current_line_width;
            current_line_width = 0;
            height += 128 * font_size;
        }
    }

    return {width, height / 2.0f}; // I have no clue why height must be divided by two...
}

//
// Button
//

UIButton::UIButton()
{
    _position = glm::vec2(0);
    _size = glm::vec2(1);
    _ClickAction = [](){};

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int image_width, image_height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    std::filesystem::path button_path = Storage::ASSET_DIR / "images" / "ui_button.png";
    unsigned char *image_data = stbi_load(button_path.c_str(), &image_width, &image_height, &num_channels, 0);
    _button_image_size = {image_width, image_height};

    int format = (num_channels == 3) ? GL_RGB : GL_RGBA; // I expect either 3 or 4 channels
    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);
}

void UIButton::SetPosition(glm::vec2 position)
{
    _position = position;
    _text.SetPosition(position);

    float desired_width = _size.x;
    float desired_height = _size.y;

    float scale = desired_height / _button_image_size.y;
    float edge_width = 40.0f * scale;
    float middle_width = desired_width - 2*edge_width;

    float uv_1 = 40.0f / _button_image_size.x; // Right of the left edge
    float uv_2 = 733.0f / _button_image_size.x; // Left of the right edge

    float vertices[] = {
    //  Left quad
    //  Position--------------------------------------------  UV--------
        position.x,              position.y,                  0.0f, 0.0f, // Bottom left
        position.x + edge_width, position.y,                  uv_1, 0.0f, // Bottom right
        position.x + edge_width, position.y + desired_height, uv_1, 1.0f, // Top right
        position.x + edge_width, position.y + desired_height, uv_1, 1.0f, // Top right
        position.x,              position.y + desired_height, 0.0f, 1.0f, // Top left
        position.x,              position.y,                  0.0f, 0.0f, // Bottom left

    //  Middle quad
    //  Position-----------------------------------------------------------  UV--------
        position.x + edge_width,                position.y,                  uv_1, 0.0f, // Bottom left
        position.x + edge_width + middle_width, position.y,                  uv_2, 0.0f, // Bottom right
        position.x + edge_width + middle_width, position.y + desired_height, uv_2, 1.0f, // Top right
        position.x + edge_width + middle_width, position.y + desired_height, uv_2, 1.0f, // Top right
        position.x + edge_width,                position.y + desired_height, uv_1, 1.0f, // Top left
        position.x + edge_width,                position.y,                  uv_1, 0.0f, // Bottom left

    //  Right quad
    //  Position-----------------------------------------------------------  UV--------
        position.x + edge_width + middle_width, position.y,                  uv_2, 0.0f, // Bottom left
        position.x + desired_width,             position.y,                  1.0f, 0.0f, // Bottom right
        position.x + desired_width,             position.y + desired_height, 1.0f, 1.0f, // Top right
        position.x + desired_width,             position.y + desired_height, 1.0f, 1.0f, // Top right
        position.x + edge_width + middle_width, position.y + desired_height, uv_2, 1.0f, // Top left
        position.x + edge_width + middle_width, position.y,                  uv_2, 0.0f, // Bottom left
    };
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
}

void UIButton::SetSize(glm::vec2 size)
{
    _size = size;

    float desired_width = size.x;
    float desired_height = size.y;

    float scale = desired_height / _button_image_size.y;
    float edge_width = 40.0f * scale;
    float middle_width = desired_width - 2*edge_width;

    float uv_1 = 40.0f / _button_image_size.x; // Right of the left edge
    float uv_2 = 733.0f / _button_image_size.x; // Left of the right edge

    float vertices[] = {
    //  Left quad
    //  Position----------------------------------------------  UV--------
        _position.x,              _position.y,                  0.0f, 0.0f, // Bottom left
        _position.x + edge_width, _position.y,                  uv_1, 0.0f, // Bottom right
        _position.x + edge_width, _position.y + desired_height, uv_1, 1.0f, // Top right
        _position.x + edge_width, _position.y + desired_height, uv_1, 1.0f, // Top right
        _position.x,              _position.y + desired_height, 0.0f, 1.0f, // Top left
        _position.x,              _position.y,                  0.0f, 0.0f, // Bottom left

    //  Middle quad
    //  Position-------------------------------------------------------------  UV--------
        _position.x + edge_width,                _position.y,                  uv_1, 0.0f, // Bottom left
        _position.x + edge_width + middle_width, _position.y,                  uv_2, 0.0f, // Bottom right
        _position.x + edge_width + middle_width, _position.y + desired_height, uv_2, 1.0f, // Top right
        _position.x + edge_width + middle_width, _position.y + desired_height, uv_2, 1.0f, // Top right
        _position.x + edge_width,                _position.y + desired_height, uv_1, 1.0f, // Top left
        _position.x + edge_width,                _position.y,                  uv_1, 0.0f, // Bottom left

    //  Right quad
    //  Position-------------------------------------------------------------  UV--------
        _position.x + edge_width + middle_width, _position.y,                  uv_2, 0.0f, // Bottom left
        _position.x + desired_width,             _position.y,                  1.0f, 0.0f, // Bottom right
        _position.x + desired_width,             _position.y + desired_height, 1.0f, 1.0f, // Top right
        _position.x + desired_width,             _position.y + desired_height, 1.0f, 1.0f, // Top right
        _position.x + edge_width + middle_width, _position.y + desired_height, uv_2, 1.0f, // Top left
        _position.x + edge_width + middle_width, _position.y,                  uv_2, 0.0f, // Bottom left
    };
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
}

void UIButton::SetText(std::string text, float font_size, glm::vec4 color)
{
    _text.SetText(text);
    _text.SetFontSize(font_size);
    _text.SetColor(color);
}

void UIButton::SetClickAction(std::function<void()> click_action)
{
    _ClickAction = click_action;
}

void UIButton::Update(MouseState mouse_state)
{
    _hovered = mouse_state.position.x >= _position.x && mouse_state.position.x <= _position.x + _size.x
            && mouse_state.position.y >= _position.y && mouse_state.position.y <= _position.y + _size.y;
    if (_hovered)
    {
        _clicked = mouse_state.left_clicked;
        _held = mouse_state.left_held;
    }
    else
    {
        _clicked = false;
        _held = false;
    }

    if (_clicked)
        _ClickAction();
}

void UIButton::Render()
{
    Shader image_shader = ShaderManager::UI_IMAGE_SHADER;
    image_shader.Use();

    if (_held)
        image_shader.SetFloat("darkness", 0.2f);
    else if (_hovered)
        image_shader.SetFloat("darkness", 0.1f);
    else
        image_shader.SetFloat("darkness", 0.0f);

    glBindVertexArray(_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glDrawArrays(GL_TRIANGLES, 0, 18); // 3 quads, 6 verts/quad

    ShaderManager::UI_TEXT_SHADER.Use();
    _text.Render();
}
