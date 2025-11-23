#ifndef UI_H
#define UI_H

#include <string>
#include <filesystem>

#include <glm/glm.hpp>
#include <stb_truetype/stb_truetype.h>

#include "shader.h"
#include "input.h"

class UIScreenText
{
    private:
        static unsigned int _atlas_texture;
        static stbtt_packedchar _packed_chars[95];
        static stbtt_aligned_quad _aligned_quads[95];
        static void _SetupFont();

    private:
        unsigned int _vao;
        unsigned int _vbo;
        std::string _text;
        float _font_size;
        glm::vec2 _position;
        glm::vec4 _color;

    public:
        UIScreenText();
        UIScreenText(std::string text, float font_size, glm::vec2 position, glm::vec4 color);
        void SetText(std::string text);
        void Render();
};

class UIScreenImage
{
    private:
        unsigned int _vao;
        unsigned int _vbo;
        unsigned int _texture;
        glm::vec2 _position;
        glm::vec2 _dimensions;
        glm::vec2 _real_image_size; // Size of underlying image (in pixels); should not be modified

    public:
        UIScreenImage(GLint filtering = GL_LINEAR);
        UIScreenImage(std::filesystem::path image_path, glm::vec2 position, glm::vec2 dimensions, GLint filtering = GL_LINEAR);
        void LoadImage(std::filesystem::path image_path, GLint filtering = GL_LINEAR);
        void SetPosition(glm::vec2 position);
        void SetDimensions(glm::vec2 dimensions);
        glm::vec2 GetDimensions();
        glm::vec2 GetImageSize() { return _real_image_size; }
        void Render();
};

class UIButton
{
    // ...
};

class UIMoonSettingsMenu
{
    // ...
};

class UIOptionsMenu
{
    // ...
};

class UIDeleteMoonMenu
{

};

class UIMainMenu
{
    private:
        UIScreenImage _lunacraft_text;
        UIScreenImage _background_images[5];
        int _current_bg = 0;
        float _current_bg_time = 0;
        // UIMoonSettingsMenu _moon_settings_menu;
        // UIDeleteMoonMenu _delete_moon_menu;
        // UIOptionsMenu _options_menu;
        // UIButton _buttons[10]; // 4 moons, 4 deletes, options, quit (there are more; just for now)
        

    public:
        UIMainMenu();
        void Update(MouseState mouse_state);
        void Render(float delta_time);
        void Rescale(glm::vec2 old_viewport, glm::vec2 new_viewport);
};

class UIPauseMenu
{

};

#endif
