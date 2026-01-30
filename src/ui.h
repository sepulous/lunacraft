#pragma once

#include <string>
#include <filesystem>
#include <functional>

#include <glm/glm.hpp>
#include <stb_truetype/stb_truetype.h>

#include "shader.h"
#include "moon_settings.h"

const float VIRTUAL_UI_WIDTH = 1920.0f;
const float VIRTUAL_UI_HEIGHT = 1080.0f;

// Menus
class UIMainMenu;
class UIGame;
class UILoadMoonMenu;
class UIResetMoonMenu;
class UIOptionsMenu;
class UIPauseMenu;
class UIDebugMenu;
class UIMoonSettingsMenu;

// UI elements
class UITextBox;
class UISlider;
class UIToggleButton;
class UIButton;
class UIImage;
class UIText;
class UIProgressBar;

// Helpers
void UIRescale();
glm::mat4 UIGetVirtualToWindow();

/////////////////////////////////////

class UIImage
{
    private:
        GLuint _vao;
        GLuint _vbo;
        GLuint _texture;
        glm::vec2 _position;
        glm::vec2 _size;
        float _aspect_ratio;
        
    public:
        UIImage(GLint filtering = GL_LINEAR);
        UIImage(std::filesystem::path image_path, glm::vec2 position, glm::vec2 size, GLint filtering = GL_LINEAR);
        void LoadImage(std::filesystem::path image_path, GLint filtering = GL_LINEAR);
        void SetPosition(glm::vec2 position);
        glm::vec2 GetPosition();
        void SetSize(glm::vec2 size, bool preserve_aspect_ratio = false);
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
        UIImage _image;
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
        bool IsClicked();
        void Update();
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
        void Update();
        void Render();
};

class UISlider
{
    private:
        bool _discrete = false;
        bool _clicked = false; // Necessary to drag only one slider
        bool _hovered = false;
        bool _held = false;
        float _value = 0.5f;
        float _value_min = 0.0f;
        float _value_max = 1.0f;
        glm::vec2 _position;
        glm::vec2 _size;
        UIImage _slider_bg_middle;    // This is a bit of a hack so sliders can be resized arbitrarily
        UIImage _slider_bg_right;     // without distorting the rounded corners
        UIImage _slider_level_left;   //
        UIImage _slider_level_middle; //
        UIImage _slider_handle;
        UIImage _slider_handle_held;
        UIText _slider_value_text;

    public:
        UISlider();
        void SetDiscrete(bool value);
        void SetValue(float value);
        float GetValue();
        void SetBounds(glm::vec2 bounds);
        void SetPosition(glm::vec2 position);
        void SetSize(glm::vec2 size);
        void Update();
        void Render();
};

class UIProgressBar
{
    private:
        float _level = 0; // in [0, 1]
        glm::vec2 _position;
        glm::vec2 _size;
        UIButton _slider_bg;
        UIButton _slider_level;

    public:
        UIProgressBar();
        void SetLevel(float value);
        void SetPosition(glm::vec2 position);
        void SetSize(glm::vec2 size);
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
        void SetText(std::string text);
        std::string GetText();
        void Update(float delta_time);
        void Render();
};

class UILoadMoonMenu
{
    private:
        bool _active = false;
        UIImage _background;
        UIText _title;
        UIProgressBar _progress_bar;
        UIText _status;
        int _stage = 0;

    public:
        UILoadMoonMenu();
        void SetActive(bool status);
        bool IsActive();
        void SetProgressLevel(float level);
        void Update();
        void Render();
};

class UIMoonSettingsMenu
{
    private:
        bool _active = false;
        bool _launch_clicked = false;
        int _moon_id = 0;
        MoonSettings _moon_settings;
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
        void SetMoonID(int moon_id);
        int GetMoonID();
        MoonSettings GetMoonSettings();
        void SetActive(bool status);
        bool IsActive();
        void SetLaunchButtonClicked(bool status);
        bool IsLaunchButtonClicked();
        void Reset();
        void Update(float delta_time);
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
        UIText _fullscreen;
        UIToggleButton _fullscreen_toggle;
        UIButton _back_button;

    public:
        UIOptionsMenu();
        void SetActive(bool status);
        bool IsActive();
        void Update();
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
        int GetMoon();
        void SetActive(bool status);
        bool IsActive();
        bool ResetClicked();
        void Update();
        void Render();
};

class UIMainMenu
{
    private:
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
        UILoadMoonMenu _load_moon_menu;

    public:
        UIMainMenu();
        void RefreshMoonButtonText();
        void ResetMoonSettings();
        void SetLoadProgressLevel(float progress);
        bool IsQuitClicked();
        bool IsLaunchButtonClicked();
        void SetLaunchButtonClicked(bool status);
        std::pair<int, MoonSettings> GetMoonData();
        void Update(float delta_time);
        void Render(float delta_time);
};

class UIPauseMenu
{
    private:
        bool _active = false;
        bool _quit_clicked = false;
        bool _resume_clicked = false;
        UIImage _background;
        UIButton _resume_button;
        UIButton _options_button;
        UIButton _quit_button;
        UIOptionsMenu _options_menu;

    public:
        UIPauseMenu();
        void SetActive(bool value);
        bool IsActive();
        bool QuitClicked();
        bool ResumeClicked();
        void Update();
        void Render();
};

struct DebugInfo
{
    int fps;
    glm::vec3 player_pos;
    glm::vec3 block_pos;
    uint64_t seed;
};

class UIDebugMenu
{
    private:
        bool _active = false;
        UIText _debug_text;

    public:
        UIDebugMenu();
        void SetActive(bool value);
        bool IsActive();
        void Update(const DebugInfo &debug_info);
        void Render();
};

class UIGame
{
    public:
        UIGame();
        UIPauseMenu &GetPauseMenu();
        UIDebugMenu &GetDebugMenu();
        void Update(const DebugInfo &debug_info);
        void Render();

    private:
        UIPauseMenu _pause_menu;
        UIDebugMenu _debug_menu;
};
