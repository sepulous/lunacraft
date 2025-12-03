#ifndef UI_H
#define UI_H

#include <string>
#include <filesystem>
#include <functional>

#include <glm/glm.hpp>
#include <stb_truetype/stb_truetype.h>

#include "shader.h"
#include "input.h"
#include "moon_settings.h"

const float VIRTUAL_UI_WIDTH = 1920.0f;
const float VIRTUAL_UI_HEIGHT = 1080.0f;

// Menus
class UIMainMenu;
class UILoadMoonMenu;
class UIResetMoonMenu;
class UIOptionsMenu;
class UIMoonSettingsMenu;

// UI elements
class UITextBox;
class UISlider;
class UIToggleButton;
class UIButton;
class UIImage;
class UIText;

/////////////////////////////////////

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

        static glm::vec2 GetTextSizeInPixels(std::string text, float font_size);
};

class UIButton
{
    private:
        GLuint _vao;
        GLuint _vbo;
        GLuint _texture;
        bool _hovered = false;
        bool _clicked = false;
        bool _held = false;
        glm::vec2 _position;
        glm::vec2 _size;
        UIText _text;
        glm::vec2 _button_image_size;
        std::function<void()> _ClickAction;

    public:
        UIButton();
        void SetImage(std::filesystem::path image_path);
        void SetPosition(glm::vec2 position);
        void SetSize(glm::vec2 size);
        void SetText(std::string text, float font_size, glm::vec4 color);
        void SetClickAction(std::function<void()> click_action);
        void Update(MouseState mouse_state);
        void Render();

        UIText& GetText() { return _text; }
};

class UIToggleButton
{
    private:
        bool _toggled = false;
        bool _hovered = false;
        glm::vec2 _position;
        glm::vec2 _size;
        UIImage _untoggled_image;
        UIImage _toggled_image;

    public:
        UIToggleButton();
        void SetToggledImage(std::filesystem::path image_path);
        void SetUntoggledImage(std::filesystem::path image_path);
        void SetPosition(glm::vec2 position);
        void SetSize(glm::vec2 size);
        void SetToggled(bool toggled);
        bool IsToggled();
        void Update(MouseState mouse_state);
        void Render();
};

class UISlider
{
    private:
        bool _discrete = false;
        bool _clicked = false; // Necessary to drag only one slider
        bool _held = false;
        float _value = 0.5f;
        float _value_min = 0.0f;
        float _value_max = 1.0f;
        glm::vec2 _position;
        glm::vec2 _size;
        UIButton _slider_bg;
        UIButton _slider_level;
        UIImage _slider_handle;
        UIImage _slider_handle_held;

    public:
        UISlider();
        void SetDiscrete(bool value);
        void SetValue(float value);
        float GetValue();
        void SetBounds(glm::vec2 bounds);
        void SetPosition(glm::vec2 position);
        void SetSize(glm::vec2 size);
        void Update(MouseState mouse_state);
        void Render();
};

class UITextBox
{
    private:
        bool _active = false;
        std::string _text_str;
        glm::vec2 _position;
        glm::vec2 _size;
        UIButton _box;
        UIText _text;
        UIText _cursor;
        float _cursor_time;

    public:
        UITextBox();
        void SetPosition(glm::vec2 position);
        void SetSize(glm::vec2 size);
        std::string GetText();
        void Update(float delta_time, MouseState mouse_state);
        void Render();
};

class UILoadMoonMenu
{
    private:
        bool _active = false;
        UIImage _background;
        UIText _title;

    public:
        UILoadMoonMenu();
        void SetActive(bool status);
        bool IsActive();
        void Update();
        void Render();
};

class UIMoonSettingsMenu
{
    private:
        bool _active = false;
        int _moon = 0;
        UIImage _background;
        UIText _title;
        UIText _tree_cover;
        UISlider _tree_cover_slider;
        UIText _roughness;
        UISlider _roughness_slider;
        UIText _wildlife;
        UISlider _wildlife_slider;
        UIText _seed;
        UITextBox _seed_textbox;
        UIText _mode_description;
        UIToggleButton _explore_button;
        UIToggleButton _creative_button;
        UIButton _launch_button;
        UIButton _back_button;

    public:
        UIMoonSettingsMenu();
        void SetMoon(int moon);
        void SetActive(bool status);
        bool IsActive();
        void Update(float delta_time, MouseState mouse_state);
        void Render();
};

class UIOptionsMenu
{
    private:
        bool _active = false;
        UIImage _background;
        UIText _title;
        UIText _sfx_volume;
        UISlider _sfx_volume_slider;
        UIText _music_volume;
        UISlider _music_volume_slider;
        UIText _sensitivity;
        UISlider _sensitivity_slider;
        UIText _render_distance;
        UISlider _render_distance_slider;
        UIText _show_gui;
        UIToggleButton _show_gui_toggle;
        UIText _show_fog;
        UIToggleButton _show_fog_toggle;
        UIText _show_debug;
        UIToggleButton _show_debug_toggle;
        UIButton _back_button;

    public:
        UIOptionsMenu();
        void SetActive(bool status);
        bool IsActive();
        void Update(MouseState mouse_state);
        void Render();
};

class UIResetMoonMenu
{
    private:
        bool _active = false;
        int _moon = 0;
        UIImage _background;
        UIText _title;
        UIButton _cancel_button;
        UIButton _reset_button;

    public:
        UIResetMoonMenu();
        void SetMoon(int moon);
        void SetActive(bool status);
        bool IsActive();
        void Update(MouseState mouse_state);
        void Render();
};

class UIMainMenu
{
    private:
        GLFWwindow *_window;
        UIImage _lunacraft_logo;
        UIImage _background_images[5];
        int _current_background = 0;
        float _current_background_time = 0;
        UIButton _moon_buttons[4];
        UIButton _reset_buttons[4];
        UIButton _options_button;
        UIButton _quit_button;
        UIMoonSettingsMenu _moon_settings_menu;
        UIOptionsMenu _options_menu;
        UIResetMoonMenu _reset_moon_menu;

    public:
        UIMainMenu(GLFWwindow *window);
        void Update(float delta_time, MouseState mouse_state);
        void Render(float delta_time);
};

#endif
