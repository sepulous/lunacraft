#ifndef UI_H
#define UI_H

#include <string>
#include <filesystem>
#include <functional>

#include <glm/glm.hpp>
#include <stb_truetype/stb_truetype.h>

#include "shader.h"
#include "input.h"

class UIImage
{
    private:
        GLuint _vao;
        GLuint _vbo;
        GLuint _texture;
        glm::vec2 _position;
        glm::vec2 _size;
        
    public:
        UIImage(GLint filtering = GL_LINEAR);
        UIImage(std::filesystem::path image_path, glm::vec2 position, glm::vec2 size, GLint filtering = GL_LINEAR);
        void LoadImage(std::filesystem::path image_path, GLint filtering = GL_LINEAR);
        void SetPosition(glm::vec2 position);
        glm::vec2 GetPosition();
        void SetSize(glm::vec2 size);
        glm::vec2 GetSize();
        void Render();
};

class UIText
{
    private:
        static GLuint _atlas_texture;
        static stbtt_packedchar _packed_chars[95];
        static stbtt_aligned_quad _aligned_quads[95];
        static void _SetupFont();

    private:
        GLuint _vao;
        GLuint _vbo;
        std::string _text;
        float _font_size;
        glm::vec2 _position;
        glm::vec4 _color;

    public:
        UIText();
        UIText(std::string text, float font_size, glm::vec2 position, glm::vec4 color);
        void SetText(std::string text);
        void SetPosition(glm::vec2 position);
        glm::vec2 GetPosition() { return _position; }
        void SetFontSize(float font_size);
        void SetColor(glm::vec4 color);
        void Render();
        static float GetFontSizeForWidth(std::string text, float width);
};

class UIButton
{
    private:
        bool _hovered = false;
        bool _clicked = false;
        bool _held = false;
        glm::vec2 _position;
        glm::vec2 _size;
        UIText _text;
        UIImage _default_image;
        UIImage _hover_image;
        UIImage _click_image;
        std::function<void()> _ClickAction;

    public:
        UIButton();
        UIButton(glm::vec2 position, glm::vec2 size, std::string text, float font_size, glm::vec4 text_color, std::function<void()> click_action);
        void SetPosition(glm::vec2 position); // TODO: Center text on image (need to know size of text in pixels to do this)
        void SetSize(glm::vec2 size);
        void SetText(std::string text, float font_size, glm::vec4 color);
        void SetClickAction(std::function<void()> click_action);
        void SetDefaultImage(std::filesystem::path default_image_path);
        void SetHoverImage(std::filesystem::path hover_image_path);
        void SetClickImage(std::filesystem::path click_image_path);
        void Update(MouseState mouse_state);
        void Render();

        UIText& GetText() { return _text; }
};

class UIMoonSettingsMenu
{
    private:
        bool _active = false;
        UIImage _background;
        UIButton _back_button;

    public:
        UIMoonSettingsMenu();
        void SetActive(bool status);
        bool IsActive();
        void Update(MouseState mouse_state);
        void Render();
};

class UIMainMenu
{
    private:
        UIImage _lunacraft_logo;
        UIImage _background_images[5];
        int _current_background = 0;
        float _current_background_time = 0;
        UIMoonSettingsMenu _moon_settings_menu;
        UIButton _moon_buttons[4];
        UIButton _reset_buttons[4];
        UIButton _options_button;
        UIButton _quit_button;

    public:
        UIMainMenu();
        void Update(MouseState mouse_state);
        void Render(float delta_time);
};

#endif
