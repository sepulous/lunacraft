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

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image/stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb_truetype/stb_truetype.h>

//
// Screen-space text
//

unsigned int UIScreenText::_atlas_texture = 0;
stbtt_packedchar UIScreenText::_packed_chars[95];
stbtt_aligned_quad UIScreenText::_aligned_quads[95];

void UIScreenText::_SetupFont()
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

    const size_t font_atlas_width = 512;
    const size_t font_atlas_height = 512;
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
        48,                           // Size of font in pixels. (Use STBTT_POINT_SIZE(fontSize) to use points) 
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

UIScreenText::UIScreenText()
{
    if (_atlas_texture == 0) // Font must be initialized
    {
        UIScreenText::_SetupFont();
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
    _position = glm::vec2(0);
    _color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

UIScreenText::UIScreenText(std::string text, float font_size, glm::vec2 position, glm::vec4 color)
{
    if (_atlas_texture == 0) // Font must be initialized
    {
        UIScreenText::_SetupFont();
    }

    _text = text;
    _font_size = font_size;
    _position = position;
    _color = color;

    int order[6] = { 0, 1, 2, 0, 2, 3 };
    float pixelScale = 2.0f / 720.0f; // TODO: Should be current window height
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
                (packedChar->x1 - packedChar->x0) * pixelScale * font_size,
                (packedChar->y1 - packedChar->y0) * pixelScale * font_size
            };

            glm::vec2 glyphBoundingBoxBottomLeft = 
            {
                localPosition.x + (packedChar->xoff * pixelScale * font_size),
                localPosition.y - (packedChar->yoff + packedChar->y1 - packedChar->y0) * pixelScale * font_size
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

            localPosition.x += packedChar->xadvance * pixelScale * font_size;
        }
        else if (ch == '\n')
        {
            localPosition.y -= 48 * pixelScale * font_size;
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

void UIScreenText::Render()
{
    Shader text_shader = ShaderManager::GetShader(SHADER_SCREEN_TEXT);
    text_shader.Use();
    glBindTexture(GL_TEXTURE_2D, _atlas_texture);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(text_shader.GetID(), "uFontAtlasTexture"), 0);

    float aspectRatio = 16.0f / 9.0f;
    glm::mat4 projectionMat = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f);
    glm::mat4 viewMat = glm::mat4(1.0f);

    viewMat = glm::translate(viewMat, {0.0f, 0.0f, 0.0f});
    viewMat = glm::rotate(viewMat, 0.0f, {1, 0, 0});
    viewMat = glm::rotate(viewMat, 0.0f, {0, 1, 0});
    viewMat = glm::rotate(viewMat, 0.0f, {0, 0, 1});
    viewMat = glm::scale(viewMat, {1.0f, 1.0f, 1.0f});

    glm::mat4 viewProjectionMat = projectionMat * viewMat;

    int uniformLocation = glGetUniformLocation(text_shader.GetID(), "uViewProjectionMat");
    glUniformMatrix4fv(uniformLocation, 1, GL_TRUE, glm::value_ptr(viewProjectionMat));

    glBindVertexArray(_vao);
    size_t vertex_count = _text.length() * 6 * (4 + 4); // # of floats in VBO
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
}

void UIScreenText::SetText(std::string text)
{
    int order[6] = { 0, 1, 2, 0, 2, 3 };
    float pixelScale = 2.0f / 720.0f; // TODO: Should be current window height
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
                (packedChar->x1 - packedChar->x0) * pixelScale * _font_size,
                (packedChar->y1 - packedChar->y0) * pixelScale * _font_size
            };

            glm::vec2 glyphBoundingBoxBottomLeft = 
            {
                localPosition.x + (packedChar->xoff * pixelScale * _font_size),
                localPosition.y - (packedChar->yoff + packedChar->y1 - packedChar->y0) * pixelScale * _font_size
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
                vertices[vertices_index + 4] = _color.r;
                vertices[vertices_index + 5] = _color.g;
                vertices[vertices_index + 6] = _color.b;
                vertices[vertices_index + 7] = _color.a;
                vertices_index += 8;
            }

            localPosition.x += packedChar->xadvance * pixelScale * _font_size;
        }
        else if (ch == '\n')
        {
            localPosition.y -= 48 * pixelScale * _font_size;
            localPosition.x = _position.x;
        }
    }

    size_t vbo_size = text.length() * 6 * (4 + 4) * sizeof(float); // (# chars) x (6 verts/char) x (3 + 4 + 2 floats/vert) x sizeof(float)
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_size, vertices, GL_DYNAMIC_DRAW);

    _text = text;
}

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

    // Vertex attribute pointer
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    _position = glm::vec2(0);
    _scale = 1.0f;
}

UIScreenImage::UIScreenImage(std::filesystem::path image_path, float x, float y, float scale)
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    int format = (num_channels == 3) ? GL_RGB : GL_RGBA; // I expect either 3 or 4 channels
    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);

    GLint viewport_info[4]; // [x, y, width, height]
    glGetIntegerv(GL_VIEWPORT, viewport_info);
    float height = 0.2f * scale;
    float width = height * ((float)viewport_info[3] / (float)viewport_info[2]) * ((float)image_width / (float)image_height);
    float vertices[] = {
    //  Position-------------  UV--------
        x,         y,          0.0f, 0.0f, // Bottom left
        x + width, y,          1.0f, 0.0f, // Bottom right
        x + width, y + height, 1.0f, 1.0f, // Top right
        x + width, y + height, 1.0f, 1.0f, // Top right
        x,         y + height, 0.0f, 1.0f, // Top left
        x,         y,          0.0f, 0.0f, // Bottom left
    };

    // Vertex buffer object
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    // Vertex attribute pointer
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    _position = glm::vec2(x, y);
    _scale = scale;
    _real_image_size = glm::vec2(image_width, image_height);
}

void UIScreenImage::SetImage(std::filesystem::path image_path)
{
    int image_width, image_height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image_data = stbi_load(image_path.c_str(), &image_width, &image_height, &num_channels, 0);

    glBindTexture(GL_TEXTURE_2D, _texture); // Already exists since constructor was called
    int format = (num_channels == 3) ? GL_RGB : GL_RGBA; // I expect either 3 or 4 channels
    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);

    _real_image_size = glm::vec2(image_width, image_height);
}

void UIScreenImage::SetPosition(float x, float y)
{
    GLint viewport_info[4]; // [x, y, width, height]
    glGetIntegerv(GL_VIEWPORT, viewport_info);
    float height = 0.2f * _scale;
    float width = height * ((float)viewport_info[3] / (float)viewport_info[2]) * ((float)_real_image_size.x / (float)_real_image_size.y);
    float vertices[] = {
    //  Position-------------  UV--------
        x,         y,          0.0f, 0.0f, // Bottom left
        x + width, y,          1.0f, 0.0f, // Bottom right
        x + width, y + height, 1.0f, 1.0f, // Top right
        x + width, y + height, 1.0f, 1.0f, // Top right
        x,         y + height, 0.0f, 1.0f, // Top left
        x,         y,          0.0f, 0.0f, // Bottom left
    };

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    _position = glm::vec2(x, y);
}

void UIScreenImage::SetScale(float scale)
{
    GLint viewport_info[4]; // [x, y, width, height]
    glGetIntegerv(GL_VIEWPORT, viewport_info);
    float height = 0.2f * scale;
    float width = height * ((float)viewport_info[3] / (float)viewport_info[2]) * ((float)_real_image_size.x / (float)_real_image_size.y);
    float vertices[] = {
    //  Position---------------------------------  UV--------
        _position.x,         _position.y,          0.0f, 0.0f, // Bottom left
        _position.x + width, _position.y,          1.0f, 0.0f, // Bottom right
        _position.x + width, _position.y + height, 1.0f, 1.0f, // Top right
        _position.x + width, _position.y + height, 1.0f, 1.0f, // Top right
        _position.x,         _position.y + height, 0.0f, 1.0f, // Top left
        _position.x,         _position.y,          0.0f, 0.0f, // Bottom left
    };

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    _scale = scale;
}

void UIScreenImage::Render()
{
    glBindVertexArray(_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

//
// Main Menu
//

UIMainMenu::UIMainMenu()
{
    _lunacraft_text.SetImage(Storage::ASSET_DIR / "images" / "lunacraft.png");
    _lunacraft_text.SetPosition(-0.95f, 0.7f);
    _lunacraft_text.SetScale(1.2f);

    std::vector<std::filesystem::path> background_image_paths = {
        Storage::ASSET_DIR / "images" / "main_menu_1.png",
        Storage::ASSET_DIR / "images" / "main_menu_2.png",
        Storage::ASSET_DIR / "images" / "main_menu_3.png",
        Storage::ASSET_DIR / "images" / "main_menu_4.png",
        Storage::ASSET_DIR / "images" / "main_menu_5.png"
    };

    GLint viewport_info[4]; // [x, y, width, height]
    glGetIntegerv(GL_VIEWPORT, viewport_info);

    //_background_images = new UIScreenImage[background_image_paths.size()];
    for (int i = 0; i < background_image_paths.size(); i++)
    {
        //_background_images[i] = UIScreenImage(background_image_paths[i], -1, -1, 0);
        _background_images[i].SetImage(background_image_paths[i]);
        _background_images[i].SetPosition(-1, -1);
        _background_images[i].SetScale(0);
        glm::vec2 bg_image_size = _background_images[i].GetImageSize();
        float scale = 10.0f; // Fill viewport height (by default)
        if (viewport_info[2] >= viewport_info[3]) // Expand to fill viewport width (if viewport is wider than it is tall)
            scale *= ((float)viewport_info[2] / (float)viewport_info[3]) * (bg_image_size.y / bg_image_size.x);
        _background_images[i].SetScale(scale);
    }

    // // _buttons[0] = UIButton(...);
    // // ...
    // // _buttons[9] = UIButton(...);
}

void UIMainMenu::Update(MouseState mouse_state)
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
    Shader screen_image_shader = ShaderManager::GetShader(SHADER_SCREEN_IMAGE);
    
    //
    // Background images
    //

    screen_image_shader.Use();

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
    _lunacraft_text.Render();
    

    // for (UIButton button : _buttons)
    //     button.Render();

    // if (_moon_settings_menu.IsActive())
    //     _moon_settings_menu.Render();

    // if (_options_menu.IsActive())
    //     _options_menu.Render();
}

void UIMainMenu::Rescale(glm::vec2 old_viewport, glm::vec2 new_viewport)
{
    float width_change_ratio = new_viewport.x / old_viewport.x;
    float height_change_ratio = new_viewport.y / old_viewport.y;

    _lunacraft_text.SetScale(_lunacraft_text.GetScale() * (width_change_ratio / height_change_ratio));

    for (int i = 0; i < 5; i++)
    {
        glm::vec2 bg_image_size = _background_images[i].GetImageSize();
        float scale = 10.0f; // Fill viewport height
        float width = 0.2f * scale * (new_viewport.y / new_viewport.x) * (bg_image_size.x / bg_image_size.y);
        float scale_factor = (width < 2.0f) ? (2.0f / width) : 1.0f;
        _background_images[i].SetScale(scale * scale_factor);

        float width_diff = (width * scale_factor) - 2.0f;
        float height_diff = (0.2f * scale * scale_factor) - 2.0f;
        _background_images[i].SetPosition(-1 - (width_diff / 2), -1 - (height_diff / 2));
    }
}
