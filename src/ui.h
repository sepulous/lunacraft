#pragma once

#include <string>
#include <filesystem>
#include <functional>

#include <glm/glm.hpp>
#include <stb_truetype/stb_truetype.h>

#include "shader.h"
#include "moon_settings.h"
#include "mesh.h"
#include "inventory.h"

class ItemStack; // Forward declarations
class Player;    //

const float VIRTUAL_UI_WIDTH = 1920.0f;
const float VIRTUAL_UI_HEIGHT = 1080.0f;

// Menus
class UIMainMenu;
class UIGame;
class UILoadMoonMenu;
class UIResetMoonMenu;
class UIOptionsMenu;

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
        Mesh quad_;
        glm::vec2 position_;
        glm::vec2 size_;
        glm::vec4 crop_;
        float aspect_ratio_;
        
    public:
        UIImage(GLint filtering = GL_LINEAR);
        void LoadImage(std::filesystem::path image_path, GLint filtering = GL_LINEAR);
        void LoadImage(unsigned char *bytes, int width, int height, int num_channels, GLint filtering = GL_LINEAR);
        void SetPosition(glm::vec2 position);
        glm::vec2 GetPosition();
        void SetSize(glm::vec2 size, bool preserve_aspect_ratio = false);
        glm::vec2 GetSize();
        void SetCrop(glm::vec4 crop);
        void Render();
};

class UIText
{
    private:
        static GLuint atlas_texture_;
        static stbtt_packedchar packed_chars_[95];
        static stbtt_aligned_quad aligned_quads_[95];
        static void SetupFont_();

    private:
        GLuint vao_;
        GLuint vbo_;
        std::string text_;
        float font_size_;
        glm::vec2 position_;
        glm::vec4 color_;

    public:
        UIText();
        UIText(std::string text, float font_size, glm::vec2 position, glm::vec4 color);
        void SetText(std::string text);
        void SetPosition(glm::vec2 position);
        glm::vec2 GetPosition() { return position_; }
        void SetFontSize(float font_size);
        void SetColor(glm::vec4 color);
        void Render();

        static glm::vec2 GetTextSizeInPixels(std::string text, float font_size);
};

class UIButton
{
    private:
        UIImage image_;
        bool hovered_ = false;
        bool clicked_ = false;
        bool held_ = false;
        glm::vec2 position_;
        glm::vec2 size_;
        UIText text_;
        glm::vec2 button_image_size_;
        std::function<void()> ClickAction_;

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

        UIText& GetText() { return text_; }
};

class UIToggleButton
{
    private:
        bool toggled_ = false;
        bool hovered_ = false;
        glm::vec2 position_;
        glm::vec2 size_;
        UIImage untoggled_image_;
        UIImage toggled_image_;

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
        bool discrete_ = false;
        bool clicked_ = false; // Necessary to drag only one slider
        bool hovered_ = false;
        bool held_ = false;
        float value_ = 0.5f;
        float value_min_ = 0.0f;
        float value_max_ = 1.0f;
        glm::vec2 position_;
        glm::vec2 size_;
        UIImage slider_bg_middle_;    // This is a bit of a hack so sliders can be resized arbitrarily
        UIImage slider_bg_right_;     // without distorting the rounded corners
        UIImage slider_level_left_;   //
        UIImage slider_level_middle_; //
        UIImage slider_handle_;
        UIImage slider_handle_held_;
        UIText slider_value_text_;

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
        float level_ = 0; // in [0, 1]
        glm::vec2 position_;
        glm::vec2 size_;
        UIButton slider_bg_;
        UIButton slider_level_;

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
        bool active_ = false;
        std::string text_str_;
        glm::vec2 position_;
        glm::vec2 size_;
        UIButton box_;
        UIText text_;
        UIText cursor_;
        float cursor_time_;

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
        bool active_ = false;
        UIImage background_;
        UIText title_;
        UIProgressBar progress_bar_;
        UIText status_;
        int stage_ = 0;

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
        bool active_ = false;
        bool launch_clicked_ = false;
        int moon_id_ = 0;
        MoonSettings moon_settings_;
        UIImage background_;
        UIText title_;
        UIText tree_cover_;
        UISlider tree_cover_slider_;
        UIText roughness_;
        UISlider roughness_slider_;
        UIText wildlife_;
        UISlider wildlife_slider_;
        UIText seed_;
        UITextBox seed_textbox_;
        UIText mode_description_;
        UIToggleButton explore_button_;
        UIToggleButton creative_button_;
        UIButton launch_button_;
        UIButton back_button_;

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
        bool active_ = false;
        UIImage background_;
        UIText title_;
        UIText sfx_volume_;
        UISlider sfx_volume_slider_;
        UIText music_volume_;
        UISlider music_volume_slider_;
        UIText sensitivity_;
        UISlider sensitivity_slider_;
        UIText render_distance_;
        UISlider render_distance_slider_;
        UIText show_gui_;
        UIToggleButton show_gui_toggle_;
        UIText show_fog_;
        UIToggleButton show_fog_toggle_;
        UIText show_debug_;
        UIToggleButton show_debug_toggle_;
        UIText fullscreen_;
        UIToggleButton fullscreen_toggle_;
        UIButton back_button_;

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
        bool active_ = false;
        int moon_ = 0;
        UIImage background_;
        UIText title_;
        UIButton cancel_button_;
        UIButton reset_button_;

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
        UIImage lunacraft_logo_;
        UIImage background_images_[5];
        int current_background_ = 0;
        float current_background_time_ = 0;
        UIButton moon_buttons_[4];
        UIButton reset_buttons_[4];
        UIButton options_button_;
        UIButton quit_button_;
        UIMoonSettingsMenu moon_settings_menu_;
        UIOptionsMenu options_menu_;
        UIResetMoonMenu reset_moon_menu_;
        UILoadMoonMenu load_moon_menu_;

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
        bool active_ = false;
        bool quit_clicked_ = false;
        bool resume_clicked_ = false;
        UIImage background_;
        UIButton resume_button_;
        UIButton options_button_;
        UIButton quit_button_;
        UIOptionsMenu options_menu_;

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
        bool active_ = false;
        UIText debug_text_;

    public:
        UIDebugMenu();
        void SetActive(bool value);
        bool IsActive();
        void Update(const DebugInfo &debug_info);
        void Render();
};

class UIInventory
{
    public:
        UIInventory();
        void Update(Player *player);
        void Render();
        bool IsActive();
        void SetActive(bool active);

    private:
        bool active_ = false;
        bool ui_built_ = false;
        Inventory local_inv_;

        UIImage hotbar_base_;
        UIImage inventory_base_;

        std::pair<UIImage, UIText> inventory_slots_[5][10];

        std::pair<UIImage, UIText> assembler_input_slots_[3][3];
        std::pair<UIImage, UIText> assembler_output_slot_;

        std::pair<UIImage, UIText> spacesuit_slots_[3];

        std::pair<UIImage, UIText> scanner_slot_;
        UIText scanner_text_;

        UIText suit_status_text_;
        UIImage suit_status_bar_;

        UIText health_text_;
        UIImage health_bar_;

        UIImage hotbar_select_;

        UIImage held_item_;
        UIText held_amount_;

        UIImage jetpack_icon_;
        UIImage jetpack_bar_bg_;
        UIImage jetpack_bar_;

        ItemStack *GetSlotUnderMouse(glm::dvec2 mouse_pos, Inventory &inventory, std::pair<UIImage, UIText> **out_slot);
        void RebuildUI(Player *player, bool force);
};

class UIGame
{
    public:
        UIGame();
        UIPauseMenu &GetPauseMenu();
        UIDebugMenu &GetDebugMenu();
        UIInventory &GetInventoryUI();
        void Update(const DebugInfo &debug_info);
        void Render();

    private:
        UIPauseMenu pause_menu_;
        UIDebugMenu debug_menu_;
        UIInventory inventory_;
        UIImage crosshair_;
};
