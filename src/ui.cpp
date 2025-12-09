#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ui.h"
#include "storage.h"
#include "input.h"
#include "shader.h"
#include "options.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image/stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb_truetype/stb_truetype.h>

extern void LoadMoon(int, MoonSettings);

int LAST_INPUT_KEY = ' ';
bool LAST_INPUT_KEY_HANDLED = true;

void HandleTextInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key >= (int)' ' && (key < 128 || key == GLFW_KEY_BACKSPACE) && action == GLFW_PRESS)
    {
        LAST_INPUT_KEY = key;
        LAST_INPUT_KEY_HANDLED = false;
    }
}

//
// Main Menu
//

UIMainMenu::UIMainMenu(GLFWwindow *window)
{
    _window = window;

    // Lunacraft logo
    _lunacraft_logo.LoadImage(Storage::IMAGE_DIR / "lunacraft.png");
    _lunacraft_logo.SetPosition({70, 875});
    _lunacraft_logo.SetSize({859, 130});

    // Background images
    int i = 1;
    for (UIImage& background_image : _background_images)
    {
        std::string image_path = std::string("main_menu_") + std::to_string(i) + ".png";
        background_image.LoadImage(Storage::IMAGE_DIR / image_path);
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

        std::string button_text;
        std::string moon_folder = Storage::MOON_DIR / (std::string("moon") + std::to_string(i));
        if (std::filesystem::exists(moon_folder))
        {
            int chunk_count = std::count_if(
                std::filesystem::directory_iterator(moon_folder),
                std::filesystem::directory_iterator{},
                [](const std::filesystem::directory_entry& e){ return e.is_regular_file(); }
            );

            if (chunk_count == 0)
            {
                button_text = std::string("Moon ") + "ABCD"[i] + " - Unexplored";
                moon_button.SetClickAction([this, i]() {
                    _moon_settings_menu.SetMoon(i);
                    _moon_settings_menu.SetActive(true);
                });
            }
            else
            {
                float distance_traveled = glm::sqrt(chunk_count);
                std::ostringstream text;
                text << "Moon " << "ABCD"[i] << " - " << std::fixed << std::setprecision(1) << distance_traveled << " Sq km";
                button_text = text.str();
                moon_button.SetClickAction([i]() { LoadMoon(i, MoonSettings()); });
            }
        }
        else
        {
            button_text = std::string("Moon ") + "ABCD"[i] + " - Unexplored";
            moon_button.SetClickAction([this, i]() {
                _moon_settings_menu.SetMoon(i);
                _moon_settings_menu.SetActive(true);
            });
        }
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
        reset_button.SetClickAction([this, i]() {
            _reset_moon_menu.SetMoon(i);
            _reset_moon_menu.SetActive(true);
        });

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
    _options_button.SetClickAction([this]() { _options_menu.SetActive(true); });

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
    _quit_button.SetClickAction([this]() { glfwSetWindowShouldClose(_window, true); });

    glfwSetKeyCallback(window, HandleTextInput);
}

void UIMainMenu::RefreshMoonButtonText()
{
    int i = 0;
    for (UIButton& moon_button : _moon_buttons)
    {
        glm::vec2 button_position = {595, 705 - i*(105 + 25)};
        glm::vec2 button_size = {820, 105};

        std::string button_text;
        std::string moon_folder = Storage::MOON_DIR / (std::string("moon") + std::to_string(i));
        if (std::filesystem::exists(moon_folder))
        {
            int chunk_count = std::count_if(
                std::filesystem::directory_iterator(moon_folder),
                std::filesystem::directory_iterator{},
                [](const std::filesystem::directory_entry& e){ return e.is_regular_file(); }
            );

            if (chunk_count == 0)
            {
                button_text = std::string("Moon ") + "ABCD"[i] + " - Unexplored";
                moon_button.SetClickAction([this, i]() {
                    _moon_settings_menu.SetMoon(i);
                    _moon_settings_menu.SetActive(true);
                });
            }
            else
            {
                float distance_traveled = glm::sqrt(chunk_count);
                std::ostringstream text;
                text << "Moon " << "ABCD"[i] << " - " << std::fixed << std::setprecision(1) << distance_traveled << " Sq km";
                button_text = text.str();
                moon_button.SetClickAction([i]() { LoadMoon(i, MoonSettings()); });
            }
        }
        else
        {
            button_text = std::string("Moon ") + "ABCD"[i] + " - Unexplored";
            moon_button.SetClickAction([this, i]() {
                _moon_settings_menu.SetMoon(i);
                _moon_settings_menu.SetActive(true);
            });
        }
        glm::vec2 text_size = UIText::GetTextSizeInPixels(button_text, 0.5f);
        moon_button.GetText().SetPosition({
            button_position.x + (button_size.x / 2.0f) - (text_size.x / 2.0f),
            button_position.y + (button_size.y / 2.0f) - (text_size.y / 2.0f)
        });
        moon_button.SetText(button_text, 0.5f, {0.0f, 0.0f, 0.0f, 1.0f});

        i++;
    }
}

void UIMainMenu::ResetMoonSettings()
{
    _moon_settings_menu.Reset();
}

void UIMainMenu::Update(float delta_time, MouseState mouse_state)
{
    if (_moon_settings_menu.IsActive())
    {
        _moon_settings_menu.Update(delta_time, mouse_state);
    }
    else if (_options_menu.IsActive())
    {
        _options_menu.Update(mouse_state);
    }
    else if (_reset_moon_menu.IsActive())
    {
        _reset_moon_menu.Update(mouse_state);
        if (_reset_moon_menu.ResetClicked())
            RefreshMoonButtonText();
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
    else if (_options_menu.IsActive())
        _options_menu.Render();
    else if (_reset_moon_menu.IsActive())
        _reset_moon_menu.Render();
}

//
// Moon Settings Menu
//

UIMoonSettingsMenu::UIMoonSettingsMenu()
{
    // Background
    _background.LoadImage(Storage::IMAGE_DIR / "ui" / "ui_menu_bg.png", GL_NEAREST);
    float bg_width = 1200;
    float bg_height = 700;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    _background.SetSize({bg_width, bg_height});
    _background.SetPosition({bg_pos_x, bg_pos_y});

    // Title
    glm::vec2 title_size = UIText::GetTextSizeInPixels("Moon Settings", 0.8f);
    glm::vec2 title_position = {bg_pos_x + (bg_width / 2.0f) - (title_size.x / 2.0f), bg_pos_y + bg_height - 100};
    _title.SetPosition(title_position);
    _title.SetText("Moon Settings");
    _title.SetFontSize(0.8f);
    _title.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    float setting_text_align_x = 580;
    float setting_font_size = 0.4f;

    // Tree cover
    float tree_text_width = UIText::GetTextSizeInPixels("Tree Cover:", setting_font_size).x;
    _tree_cover.SetPosition({bg_pos_x + setting_text_align_x - tree_text_width, 700});
    _tree_cover.SetText("Tree Cover:");
    _tree_cover.SetFontSize(setting_font_size);
    _tree_cover.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Tree cover slider
    _tree_cover_slider.SetDiscrete(true);
    _tree_cover_slider.SetPosition({bg_pos_x + setting_text_align_x + 20, 700 + 6});
    _tree_cover_slider.SetSize({460, 20});
    _tree_cover_slider.SetBounds({0.0f, 4.0f});
    _tree_cover_slider.SetValue(2.0f);

    // Terrain roughness
    float roughness_text_width = UIText::GetTextSizeInPixels("Terrain Roughness:", setting_font_size).x;
    _roughness.SetPosition({bg_pos_x + setting_text_align_x - roughness_text_width, 640});
    _roughness.SetText("Terrain Roughness:");
    _roughness.SetFontSize(setting_font_size);
    _roughness.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Terrain roughness slider
    _roughness_slider.SetDiscrete(true);
    _roughness_slider.SetPosition({bg_pos_x + setting_text_align_x + 20, 640 + 6});
    _roughness_slider.SetSize({460, 20});
    _roughness_slider.SetBounds({0.0f, 4.0f});
    _roughness_slider.SetValue(2.0f);

    // Wildlife level
    float wildlife_text_width = UIText::GetTextSizeInPixels("Wildlife Level:", setting_font_size).x;
    _wildlife.SetPosition({bg_pos_x + setting_text_align_x - wildlife_text_width, 580});
    _wildlife.SetText("Wildlife Level:");
    _wildlife.SetFontSize(setting_font_size);
    _wildlife.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Wildlife level slider
    _wildlife_slider.SetDiscrete(true);
    _wildlife_slider.SetPosition({bg_pos_x + setting_text_align_x + 20, 580 + 6});
    _wildlife_slider.SetSize({460, 20});
    _wildlife_slider.SetBounds({0.0f, 4.0f});
    _wildlife_slider.SetValue(2.0f);

    // Seed
    float seed_text_width = UIText::GetTextSizeInPixels("Seed:", setting_font_size).x;
    _seed.SetPosition({bg_pos_x + setting_text_align_x - seed_text_width, 520});
    _seed.SetText("Seed:");
    _seed.SetFontSize(setting_font_size);
    _seed.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Seed textbox
    _seed_textbox.SetPosition({bg_pos_x + setting_text_align_x + 20, 505});
    _seed_textbox.SetSize({460, 50});

    // Mode description
    float mode_text_width = UIText::GetTextSizeInPixels("Survive on an alien moon", 0.3f).x;
    _mode_description.SetPosition({bg_pos_x + (bg_width / 2.0f) - (mode_text_width / 2.0f), bg_pos_y + 230});
    _mode_description.SetText("Survive on an alien moon");
    _mode_description.SetFontSize(0.3f);
    _mode_description.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Creative button
    _creative_button.SetToggledImage(Storage::IMAGE_DIR / "ui" / "creative_toggled.png");
    _creative_button.SetUntoggledImage(Storage::IMAGE_DIR / "ui" / "creative_untoggled.png");
    glm::vec2 creative_button_size = {200, 60};
    _creative_button.SetSize(creative_button_size);
    _creative_button.SetPosition({bg_pos_x + (bg_width / 2.0f) - (creative_button_size.x / 2.0f) - 100, bg_pos_y + 150});

    // Explore button
    _explore_button.SetToggledImage(Storage::IMAGE_DIR / "ui" / "explore_toggled.png");
    _explore_button.SetUntoggledImage(Storage::IMAGE_DIR / "ui" / "explore_untoggled.png");
    glm::vec2 explore_button_size = {200, 60};
    _explore_button.SetSize(explore_button_size);
    _explore_button.SetPosition({bg_pos_x + (bg_width / 2.0f) + (explore_button_size.x / 2.0f) - 100, bg_pos_y + 150});
    _explore_button.SetToggled(true);

    // Back button
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

    // Launch button
    glm::vec2 launch_button_size = {240, 80};
    glm::vec2 launch_button_position = {bg_pos_x + (bg_width / 2.0f) - (launch_button_size.x / 2.0f), bg_pos_y + 40};
    float launch_font_size = 0.4f;
    _launch_button.SetPosition(launch_button_position);
    _launch_button.SetSize(launch_button_size);
    _launch_button.SetText("Launch!", launch_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 launch_text_size = UIText::GetTextSizeInPixels("Launch!", launch_font_size);
    _launch_button.GetText().SetPosition({
        launch_button_position.x + (launch_button_size.x / 2.0f) - (launch_text_size.x / 2.0f),
        launch_button_position.y + (launch_button_size.y / 2.0f) - (launch_text_size.y / 2.0f)
    });
    _launch_button.SetClickAction([this]() {
        MoonSettings moon_settings;
        moon_settings.tree_cover = (uint8_t)_tree_cover_slider.GetValue();
        moon_settings.terrain_roughness = (uint8_t)_roughness_slider.GetValue();
        moon_settings.wildlife_level = (uint8_t)_wildlife_slider.GetValue();
        moon_settings.is_creative = _creative_button.IsToggled();

        std::string seed_text = _seed_textbox.GetText();
        uint64_t seed_hash = 1469598103934665603ULL;
        for (unsigned char c : seed_text)
        {
            seed_hash ^= c;
            seed_hash *= 1099511628211ULL; // FNV prime
        }
        moon_settings.seed = seed_hash;

        SetActive(false);
        LoadMoon(_moon, moon_settings);
    });
}

void UIMoonSettingsMenu::SetMoon(int moon)
{
    _moon = moon;
}

void UIMoonSettingsMenu::SetActive(bool status)
{
    _active = status;
}

bool UIMoonSettingsMenu::IsActive()
{
    return _active;
}

void UIMoonSettingsMenu::Reset()
{
    // Reset sliders and seed textbox
    _tree_cover_slider.SetValue(2.0f);
    _roughness_slider.SetValue(2.0f);
    _wildlife_slider.SetValue(2.0f);
    _seed_textbox.SetText("");
    
    // Reset gamemode stuff
    float bg_width = 1200;
    float bg_height = 700;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    float mode_text_width = UIText::GetTextSizeInPixels("Survive on an alien moon", 0.3f).x;
    _mode_description.SetPosition({bg_pos_x + (bg_width / 2.0f) - (mode_text_width / 2.0f), bg_pos_y + 230});
    _mode_description.SetText("Survive on an alien moon");
    _explore_button.SetToggled(true);
    _creative_button.SetToggled(false);
}

void UIMoonSettingsMenu::Update(float delta_time, MouseState mouse_state)
{
    _back_button.Update(mouse_state);
    _launch_button.Update(mouse_state);
    _tree_cover_slider.Update(mouse_state);
    _roughness_slider.Update(mouse_state);
    _wildlife_slider.Update(mouse_state);
    _seed_textbox.Update(delta_time, mouse_state);

    bool explore_was_toggled = _explore_button.IsToggled();
    _explore_button.Update(mouse_state);
    if (explore_was_toggled && !_explore_button.IsToggled()) // Prevent both buttons being untoggled
        _explore_button.SetToggled(true);
    if (_explore_button.IsToggled())
        _creative_button.SetToggled(false);

    bool creative_was_toggled = _creative_button.IsToggled();
    _creative_button.Update(mouse_state);
    if (creative_was_toggled && !_creative_button.IsToggled()) // Prevent both buttons being untoggled
        _creative_button.SetToggled(true);
    if (_creative_button.IsToggled())
        _explore_button.SetToggled(false);

    glm::vec2 bg_pos = _background.GetPosition();
    glm::vec2 bg_size = _background.GetSize();
    float mode_text_width;
    if (_explore_button.IsToggled())
    {
        mode_text_width = UIText::GetTextSizeInPixels("Survive on an alien moon", 0.3f).x;
        _mode_description.SetPosition({bg_pos.x + (bg_size.x / 2.0f) - (mode_text_width / 2.0f), bg_pos.y + 230});
        _mode_description.SetText("Survive on an alien moon");
    }
    else
    {
        mode_text_width = UIText::GetTextSizeInPixels("Create without limits, safely", 0.3f).x;
        _mode_description.SetPosition({bg_pos.x + (bg_size.x / 2.0f) - (mode_text_width / 2.0f), bg_pos.y + 230});
        _mode_description.SetText("Create without limits, safely");
    }
}

void UIMoonSettingsMenu::Render()
{
    _background.Render();
    _title.Render();
    _tree_cover.Render();
    _tree_cover_slider.Render();
    _roughness.Render();
    _roughness_slider.Render();
    _wildlife.Render();
    _wildlife_slider.Render();
    _seed.Render();
    _seed_textbox.Render();
    _explore_button.Render();
    _creative_button.Render();
    _mode_description.Render();
    _back_button.Render();
    _launch_button.Render();
}

//
// Load Moon Menu
//

UILoadMoonMenu::UILoadMoonMenu()
{
    // Background
    _background.LoadImage(Storage::IMAGE_DIR / "ui" / "ui_menu_bg.png", GL_NEAREST);
    float bg_width = 780;
    float bg_height = 420;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    _background.SetSize({bg_width, bg_height});
    _background.SetPosition({bg_pos_x, bg_pos_y});

    // Title
    glm::vec2 title_size = UIText::GetTextSizeInPixels("Inserting astronaut...", 0.4f);
    glm::vec2 title_position = {bg_pos_x + (bg_width / 2.0f) - (title_size.x / 2.0f), bg_pos_y + bg_height - 100};
    _title.SetPosition(title_position);
    _title.SetText("Inserting astronaut...");
    _title.SetFontSize(0.4f);
    _title.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Progress bar
    glm::vec2 progress_bar_size = {400, 20};
    _progress_bar.SetPosition({bg_pos_x + (bg_width / 2.0f) - (progress_bar_size.x / 2.0f), bg_pos_y + bg_height - 200});
    _progress_bar.SetSize(progress_bar_size);

    // Status text
    glm::vec2 status_size = UIText::GetTextSizeInPixels("Loading chunks...", 0.4f);
    glm::vec2 status_position = {bg_pos_x + (bg_width / 2.0f) - (status_size.x / 2.0f), bg_pos_y + bg_height - 300};
    _status.SetPosition(status_position);
    _status.SetText("Loading chunks...");
    _status.SetFontSize(0.4f);
    _status.SetColor({0.0f, 0.0f, 0.0f, 1.0f});
}

void UILoadMoonMenu::SetActive(bool status)
{
    _active = status;
}

bool UILoadMoonMenu::IsActive()
{
    return _active;
}

void UILoadMoonMenu::SetProgressLevel(float level)
{
    _progress_bar.SetLevel(level);
    int stage = level < 0.5f ? 0 : 1;
    if (stage != _stage)
    {
        float bg_width = 780;
        float bg_height = 420;
        float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
        float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
        if (stage == 0) // Loading chunks...
        {
            glm::vec2 status_size = UIText::GetTextSizeInPixels("Loading chunks...", 0.4f);
            glm::vec2 status_position = {bg_pos_x + (bg_width / 2.0f) - (status_size.x / 2.0f), bg_pos_y + bg_height - 300};
            _status.SetPosition(status_position);
            _status.SetText("Loading chunks...");
        }
        else // Building geometry...
        {
            glm::vec2 status_size = UIText::GetTextSizeInPixels("Building geometry...", 0.4f);
            glm::vec2 status_position = {bg_pos_x + (bg_width / 2.0f) - (status_size.x / 2.0f), bg_pos_y + bg_height - 300};
            _status.SetPosition(status_position);
            _status.SetText("Building geometry...");
        }

        _stage = stage;
    }
}

void UILoadMoonMenu::Update()
{
    
}

void UILoadMoonMenu::Render()
{
    _background.Render();
    _title.Render();
    _progress_bar.Render();
    _status.Render();
}

//
// Options Menu
//

UIOptionsMenu::UIOptionsMenu()
{
    // Background
    _background.LoadImage(Storage::IMAGE_DIR / "ui" / "ui_menu_bg.png", GL_NEAREST);
    float bg_width = 1200;
    float bg_height = 700;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    _background.SetSize({bg_width, bg_height});
    _background.SetPosition({bg_pos_x, bg_pos_y});

    // Title
    glm::vec2 title_size = UIText::GetTextSizeInPixels("Options", 0.8f);
    glm::vec2 title_position = {bg_pos_x + (bg_width / 2.0f) - (title_size.x / 2.0f), bg_pos_y + bg_height - 100};
    _title.SetPosition(title_position);
    _title.SetText("Options");
    _title.SetFontSize(0.8f);
    _title.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    Options current_options = OptionsManager::GetOptions();

    float option_text_align_x1 = 360;
    float option_text_align_x2 = 1050;
    float option_font_size = 0.4f;

    // SFX volume
    float sfx_text_width = UIText::GetTextSizeInPixels("SFX Vol:", option_font_size).x;
    _sfx_volume.SetPosition({bg_pos_x + option_text_align_x1 - sfx_text_width, 680});
    _sfx_volume.SetText("SFX Vol:");
    _sfx_volume.SetFontSize(option_font_size);
    _sfx_volume.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // SFX volume slider
    _sfx_volume_slider.SetPosition({bg_pos_x + option_text_align_x1 + 25, 685});
    _sfx_volume_slider.SetSize({240, 20});
    _sfx_volume_slider.SetBounds({0.0f, 1.0f});
    _sfx_volume_slider.SetValue(current_options.sfx_volume);

    // Music volume
    float music_text_width = UIText::GetTextSizeInPixels("Music Vol:", option_font_size).x;
    _music_volume.SetPosition({bg_pos_x + option_text_align_x1 - music_text_width, 600});
    _music_volume.SetText("Music Vol:");
    _music_volume.SetFontSize(option_font_size);
    _music_volume.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Music volume slider
    _music_volume_slider.SetPosition({bg_pos_x + option_text_align_x1 + 25, 605});
    _music_volume_slider.SetSize({240, 20});
    _music_volume_slider.SetBounds({0.0f, 1.0f});
    _music_volume_slider.SetValue(current_options.music_volume);

    // Sensitivity
    float sensitivity_text_width = UIText::GetTextSizeInPixels("Ctrl Sense:", option_font_size).x;
    _sensitivity.SetPosition({bg_pos_x + option_text_align_x1 - sensitivity_text_width, 520});
    _sensitivity.SetText("Ctrl Sense:");
    _sensitivity.SetFontSize(option_font_size);
    _sensitivity.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Sensitivity slider
    _sensitivity_slider.SetPosition({bg_pos_x + option_text_align_x1 + 25, 525});
    _sensitivity_slider.SetSize({240, 20});
    _sensitivity_slider.SetBounds({0.0f, 2.0f});
    _sensitivity_slider.SetValue(current_options.sensitivity);

    // Render distance
    float render_distance_text_width = UIText::GetTextSizeInPixels("View Dist:", option_font_size).x;
    _render_distance.SetPosition({bg_pos_x + option_text_align_x1 - render_distance_text_width, 440});
    _render_distance.SetText("View Dist:");
    _render_distance.SetFontSize(option_font_size);
    _render_distance.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Render distance slider
    _render_distance_slider.SetDiscrete(true);
    _render_distance_slider.SetPosition({bg_pos_x + option_text_align_x1 + 25, 445});
    _render_distance_slider.SetSize({240, 20});
    _render_distance_slider.SetBounds({1.0f, 16.0f});
    _render_distance_slider.SetValue(current_options.render_distance);

    // Show GUI
    float gui_text_width = UIText::GetTextSizeInPixels("Show GUI:", option_font_size).x;
    _show_gui.SetPosition({bg_pos_x + option_text_align_x2 - gui_text_width, 680});
    _show_gui.SetText("Show GUI:");
    _show_gui.SetFontSize(option_font_size);
    _show_gui.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Show GUI toggle
    _show_gui_toggle.SetPosition({bg_pos_x + option_text_align_x2 + 20, 680 - 10});
    _show_gui_toggle.SetSize({40, 40});
    _show_gui_toggle.SetToggled(current_options.show_gui);

    // Show fog
    float fog_text_width = UIText::GetTextSizeInPixels("Show Fog:", option_font_size).x;
    _show_fog.SetPosition({bg_pos_x + option_text_align_x2 - fog_text_width, 600});
    _show_fog.SetText("Show Fog:");
    _show_fog.SetFontSize(option_font_size);
    _show_fog.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Show fog toggle
    _show_fog_toggle.SetPosition({bg_pos_x + option_text_align_x2 + 20, 600 - 10});
    _show_fog_toggle.SetSize({40, 40});
    _show_fog_toggle.SetToggled(current_options.show_fog);

    // Show debug info
    float debug_text_width = UIText::GetTextSizeInPixels("Debug Info:", option_font_size).x;
    _show_debug.SetPosition({bg_pos_x + option_text_align_x2 - debug_text_width, 520});
    _show_debug.SetText("Debug Info:");
    _show_debug.SetFontSize(option_font_size);
    _show_debug.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Show debug toggle
    _show_debug_toggle.SetPosition({bg_pos_x + option_text_align_x2 + 20, 520 - 10});
    _show_debug_toggle.SetSize({40, 40});
    _show_debug_toggle.SetToggled(current_options.show_debug_info);

    // Back button
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
    _back_button.SetClickAction([this]() {
        Options new_options;
        new_options.music_volume = _music_volume_slider.GetValue();
        new_options.render_distance = _render_distance_slider.GetValue();
        new_options.sensitivity = _sensitivity_slider.GetValue();
        new_options.sfx_volume = _sfx_volume_slider.GetValue();
        new_options.show_debug_info = _show_debug_toggle.IsToggled();
        new_options.show_fog = _show_fog_toggle.IsToggled();
        new_options.show_gui = _show_gui_toggle.IsToggled();
        OptionsManager::SetOptions(new_options);
        _active = false;
    });
}

void UIOptionsMenu::SetActive(bool status)
{
    _active = status;
}

bool UIOptionsMenu::IsActive()
{
    return _active;
}

void UIOptionsMenu::Update(MouseState mouse_state)
{
    _back_button.Update(mouse_state);
    _sfx_volume_slider.Update(mouse_state);
    _music_volume_slider.Update(mouse_state);
    _sensitivity_slider.Update(mouse_state);
    _render_distance_slider.Update(mouse_state);
    _show_gui_toggle.Update(mouse_state);
    _show_fog_toggle.Update(mouse_state);
    _show_debug_toggle.Update(mouse_state);
}

void UIOptionsMenu::Render()
{
    _background.Render();
    _title.Render();
    _sfx_volume.Render();
    _sfx_volume_slider.Render();
    _music_volume.Render();
    _music_volume_slider.Render();
    _sensitivity.Render();
    _sensitivity_slider.Render();
    _render_distance.Render();
    _render_distance_slider.Render();
    _show_gui.Render();
    _show_gui_toggle.Render();
    _show_fog.Render();
    _show_fog_toggle.Render();
    _show_debug.Render();
    _show_debug_toggle.Render();
    _back_button.Render();
}

//
// Reset Moon Menu
//

UIResetMoonMenu::UIResetMoonMenu()
{
    // Background
    _background.LoadImage(Storage::IMAGE_DIR / "ui" / "ui_menu_bg.png", GL_NEAREST);
    float bg_width = 720;
    float bg_height = 420;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    _background.SetSize({bg_width, bg_height});
    _background.SetPosition({bg_pos_x, bg_pos_y});

    // Title
    glm::vec2 title_size = UIText::GetTextSizeInPixels("Reset saved world?", 0.4f);
    glm::vec2 title_position = {bg_pos_x + (bg_width / 2.0f) - (title_size.x / 2.0f), bg_pos_y + bg_height - 100};
    _title.SetPosition(title_position);
    _title.SetText("Reset saved world?");
    _title.SetFontSize(0.4f);
    _title.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Cancel button
    glm::vec2 cancel_button_position = {bg_pos_x + 100, bg_pos_y + 100};
    glm::vec2 cancel_button_size = {240, 80};
    float cancel_font_size = 0.4f;
    _cancel_button.SetPosition(cancel_button_position);
    _cancel_button.SetSize(cancel_button_size);
    _cancel_button.SetText("Cancel", cancel_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 cancel_text_size = UIText::GetTextSizeInPixels("Cancel", cancel_font_size);
    _cancel_button.GetText().SetPosition({
        cancel_button_position.x + (cancel_button_size.x / 2.0f) - (cancel_text_size.x / 2.0f),
        cancel_button_position.y + (cancel_button_size.y / 2.0f) - (cancel_text_size.y / 2.0f)
    });
    _cancel_button.SetClickAction([this]() { _active = false; });

    // Reset button
    glm::vec2 reset_button_position = {bg_pos_x + 380, bg_pos_y + 100};
    glm::vec2 reset_button_size = {240, 80};
    float reset_font_size = 0.4f;
    _reset_button.SetPosition(reset_button_position);
    _reset_button.SetSize(reset_button_size);
    _reset_button.SetText("Reset", reset_font_size, {0.9f, 0.0f, 0.0f, 1.0f});
    glm::vec2 reset_text_size = UIText::GetTextSizeInPixels("Reset", reset_font_size);
    _reset_button.GetText().SetPosition({
        reset_button_position.x + (reset_button_size.x / 2.0f) - (reset_text_size.x / 2.0f),
        reset_button_position.y + (reset_button_size.y / 2.0f) - (reset_text_size.y / 2.0f)
    });
    _reset_button.SetClickAction([this]() {
        std::filesystem::path moon_path = Storage::MOON_DIR / (std::string("moon") + std::to_string(_moon));
        std::filesystem::remove_all(moon_path);
        SetActive(false);
    });
}

void UIResetMoonMenu::SetMoon(int moon)
{
    _moon = moon;
}

int UIResetMoonMenu::GetMoon()
{
    return _moon;
}

void UIResetMoonMenu::SetActive(bool status)
{
    _active = status;
}

bool UIResetMoonMenu::IsActive()
{
    return _active;
}

bool UIResetMoonMenu::ResetClicked()
{
    return _reset_button.IsClicked();
}

void UIResetMoonMenu::Update(MouseState mouse_state)
{
    _cancel_button.Update(mouse_state);
    _reset_button.Update(mouse_state);
}

void UIResetMoonMenu::Render()
{
    _background.Render();
    _title.Render();
    _cancel_button.Render();
    _reset_button.Render();
}

//
// Pause Menu
//

UIPauseMenu::UIPauseMenu()
{
    _background.LoadImage(Storage::IMAGE_DIR / "ui" / "ui_black.png", GL_NEAREST);
    _background.SetPosition({0, 0});
    _background.SetSize({VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});

    glm::vec2 button_size = {520, 120};
    float text_scale = 0.6f;

    glm::vec2 resume_button_position = {(VIRTUAL_UI_WIDTH / 2.0f) - (button_size.x / 2.0f), 700};
    _resume_button.SetSize(button_size);
    _resume_button.SetPosition(resume_button_position);
    _resume_button.SetText("Resume", text_scale, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 resume_text_size = UIText::GetTextSizeInPixels("Resume", text_scale);
    _resume_button.GetText().SetPosition({
        resume_button_position.x + (button_size.x / 2.0f) - (resume_text_size.x / 2.0f),
        resume_button_position.y + (button_size.y / 2.0f) - (resume_text_size.y / 2.0f)
    });
    _resume_button.SetClickAction([this]() { SetActive(false); });

    glm::vec2 options_button_position = {(VIRTUAL_UI_WIDTH / 2.0f) - (button_size.x / 2.0f), 500};
    _options_button.SetSize(button_size);
    _options_button.SetPosition(options_button_position);
    _options_button.SetText("Options", text_scale, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 options_text_size = UIText::GetTextSizeInPixels("Options", text_scale);
    _options_button.GetText().SetPosition({
        options_button_position.x + (button_size.x / 2.0f) - (options_text_size.x / 2.0f),
        options_button_position.y + (button_size.y / 2.0f) - (options_text_size.y / 2.0f)
    });
    _options_button.SetClickAction([this]() { _options_menu.SetActive(true); });

    glm::vec2 quit_button_position = {(VIRTUAL_UI_WIDTH / 2.0f) - (button_size.x / 2.0f), 300};
    _quit_button.SetSize(button_size);
    _quit_button.SetPosition(quit_button_position);
    _quit_button.SetText("Save & Quit", text_scale, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 quit_text_size = UIText::GetTextSizeInPixels("Save & Quit", text_scale);
    _quit_button.GetText().SetPosition({
        quit_button_position.x + (button_size.x / 2.0f) - (quit_text_size.x / 2.0f),
        quit_button_position.y + (button_size.y / 2.0f) - (quit_text_size.y / 2.0f)
    });
}

void UIPauseMenu::SetActive(bool value)
{
    _active = value;
}

bool UIPauseMenu::IsActive()
{
    return _active;
}

bool UIPauseMenu::QuitClicked()
{
    return _quit_clicked;
}

bool UIPauseMenu::ResumeClicked()
{
    return _resume_clicked;
}

void UIPauseMenu::Update(MouseState mouse_state)
{
    if (_options_menu.IsActive())
    {
        _options_menu.Update(mouse_state);
    }
    else
    {
        _resume_button.Update(mouse_state);
        _options_button.Update(mouse_state);
        _quit_button.Update(mouse_state);
        _resume_clicked = _resume_button.IsClicked();
        _quit_clicked = _quit_button.IsClicked();
    }
}

void UIPauseMenu::Render()
{
    Shader image_shader = ShaderManager::UI_IMAGE_SHADER;
    image_shader.Use();

    image_shader.SetFloat("opacity", 0.4f);
    _background.Render();
    image_shader.SetFloat("opacity", 1.0f);

    _resume_button.Render();
    _options_button.Render();
    _quit_button.Render();
    if (_options_menu.IsActive())
        _options_menu.Render();
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
    std::filesystem::path button_path = Storage::IMAGE_DIR / "ui" / "ui_button.png";
    unsigned char *image_data = stbi_load(button_path.c_str(), &image_width, &image_height, &num_channels, 0);
    _button_image_size = {image_width, image_height};

    int format = (num_channels == 3) ? GL_RGB : GL_RGBA; // I expect either 3 or 4 channels
    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);
}

void UIButton::SetImage(std::filesystem::path image_path)
{
    int image_width, image_height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image_data = stbi_load(image_path.c_str(), &image_width, &image_height, &num_channels, 0);
    _button_image_size = {image_width, image_height};

    int format = (num_channels == 3) ? GL_RGB : GL_RGBA; // I expect either 3 or 4 channels
    glBindTexture(GL_TEXTURE_2D, _texture);
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

bool UIButton::IsClicked()
{
    return _clicked;
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

//
// Toggle Button
//

UIToggleButton::UIToggleButton()
{
    _toggled_image.LoadImage(Storage::IMAGE_DIR / "ui" / "ui_toggle_checked.png");
    _untoggled_image.LoadImage(Storage::IMAGE_DIR / "ui" / "ui_toggle_unchecked.png");
}

void UIToggleButton::SetToggledImage(std::filesystem::path image_path)
{
    _toggled_image.LoadImage(image_path);
}

void UIToggleButton::SetUntoggledImage(std::filesystem::path image_path)
{
    _untoggled_image.LoadImage(image_path);
}

void UIToggleButton::SetPosition(glm::vec2 position)
{
    _position = position;
    _toggled_image.SetPosition(position);
    _untoggled_image.SetPosition(position);
}

void UIToggleButton::SetSize(glm::vec2 size)
{
    _size = size;
    _toggled_image.SetSize(size);
    _untoggled_image.SetSize(size);
}

void UIToggleButton::SetToggled(bool toggled)
{
    _toggled = toggled;
}

bool UIToggleButton::IsToggled()
{
    return _toggled;
}

void UIToggleButton::Update(MouseState mouse_state)
{
    _hovered = mouse_state.position.x >= _position.x && mouse_state.position.x <= _position.x + _size.x && mouse_state.position.y >= _position.y && mouse_state.position.y <= _position.y + _size.y;
    if (_hovered && mouse_state.left_clicked)
        _toggled = !_toggled;
}

void UIToggleButton::Render()
{
    Shader image_shader = ShaderManager::UI_IMAGE_SHADER;
    image_shader.Use();

    if (_hovered)
        image_shader.SetFloat("darkness", 0.1f);
    else
        image_shader.SetFloat("darkness", 0.0f);

    if (_toggled)
        _toggled_image.Render();
    else
        _untoggled_image.Render();
}

//
// Slider
//

UISlider::UISlider()
{
    _slider_bg.SetImage(Storage::IMAGE_DIR / "ui" / "ui_slider_bg.png");
    _slider_level.SetImage(Storage::IMAGE_DIR / "ui" / "ui_slider_level.png");
    _slider_handle.LoadImage(Storage::IMAGE_DIR / "ui" / "ui_slider_handle.png");
    _slider_handle_held.LoadImage(Storage::IMAGE_DIR / "ui" / "ui_slider_handle_held.png");
    _slider_value_text.SetFontSize(0.5f);
    _slider_value_text.SetColor({0.0f, 0.0f, 0.0f, 1.0f});
}

void UISlider::SetValue(float level)
{
    _value = glm::clamp(level, _value_min, _value_max);
    if (_discrete)
        _value = glm::round(_value);
    float f = (_value - _value_min) / (_value_max - _value_min);
    float handle_pos_x = _position.x + f*_size.x - 20;
    _slider_level.SetSize({handle_pos_x - _position.x + 20, _size.y});
    _slider_handle.SetPosition({handle_pos_x, _position.y - 20});
    _slider_handle_held.SetPosition({handle_pos_x, _position.y - 20});
    _slider_value_text.SetPosition({handle_pos_x + 30, _position.y + 30});
    if (_discrete)
    {
        _slider_value_text.SetText(std::to_string((int)_value));
    }
    else
    {
        std::ostringstream value_text;
        value_text << std::fixed << std::setprecision(2) << _value;
        _slider_value_text.SetText(value_text.str());
    }
}

float UISlider::GetValue()
{
    return _value;
}

void UISlider::SetBounds(glm::vec2 bounds)
{
    _value_min = bounds.x;
    _value_max = bounds.y;
}

void UISlider::SetPosition(glm::vec2 position)
{
    _position = position;
    _slider_bg.SetPosition(position);
    _slider_level.SetPosition(position);
    _slider_handle.SetPosition({position.x - 20, position.y - 20});
    _slider_handle_held.SetPosition({position.x - 20, position.y - 20});
}

void UISlider::SetSize(glm::vec2 size)
{
    _size = size;
    _slider_bg.SetSize(size);
    _slider_level.SetSize(size);
    float handle_size = size.y * 2.5f;
    _slider_handle.SetSize({handle_size, handle_size});
    _slider_handle_held.SetSize({handle_size, handle_size});
}

void UISlider::SetDiscrete(bool value)
{
    _discrete = value;
}

void UISlider::Update(MouseState mouse_state)
{
    glm::vec2 mouse_pos = mouse_state.position;
    glm::vec2 handle_pos = _slider_handle.GetPosition();
    glm::vec2 handle_size = _slider_handle.GetSize();
    bool mouse_on_slider = mouse_pos.x >= _position.x - 20 && mouse_pos.x <= _position.x + _size.x + 20 && mouse_pos.y >= handle_pos.y && mouse_pos.y <= handle_pos.y + handle_size.y;

    if (_clicked)
        _clicked = mouse_state.left_held;
    else
        _clicked = mouse_state.left_clicked && mouse_on_slider;

    if (_held)
        _held = mouse_state.left_held;
    else
        _held = _clicked && mouse_state.left_held && mouse_on_slider;

    if (_held) // Still held; drag slider
    {
        _value = glm::clamp(
            ((mouse_pos.x - _position.x) / _size.x) * (_value_max - _value_min) + _value_min,
            _value_min,
            _value_max
        );
        if (_discrete)
            _value = glm::round(_value);
        float f = (_value - _value_min) / (_value_max - _value_min);
        float handle_pos_x = _position.x + f*_size.x - 20;
        _slider_level.SetSize({handle_pos_x - _position.x + 20, _size.y});
        _slider_handle.SetPosition({handle_pos_x, _position.y - 20});
        _slider_handle_held.SetPosition({handle_pos_x, _position.y - 20});
        _slider_value_text.SetPosition({handle_pos_x + 30, _position.y + 30});
        if (_discrete)
        {
            _slider_value_text.SetText(std::to_string((int)_value));
        }
        else
        {
            std::ostringstream value_text;
            value_text << std::fixed << std::setprecision(2) << _value;
            _slider_value_text.SetText(value_text.str());
        }
    }
}

void UISlider::Render()
{
    _slider_bg.Render();
    _slider_level.Render();
    if (_held)
    {
        _slider_handle_held.Render();
        _slider_value_text.Render();
    }
    else
    {
        _slider_handle.Render();
    }
}

//
// Progress Bar
//

UIProgressBar::UIProgressBar()
{
    _slider_bg.SetImage(Storage::IMAGE_DIR / "ui" / "ui_slider_bg.png");
    _slider_level.SetImage(Storage::IMAGE_DIR / "ui" / "ui_slider_level.png");
}

void UIProgressBar::SetLevel(float level)
{
    _level = glm::clamp(level, 0.0f, 1.0f);
    _slider_level.SetSize({_level * _size.x, _size.y});
}

void UIProgressBar::SetPosition(glm::vec2 position)
{
    _position = position;
    _slider_bg.SetPosition(position);
    _slider_level.SetPosition(position);
}

void UIProgressBar::SetSize(glm::vec2 size)
{
    _size = size;
    _slider_bg.SetSize(size);
    _slider_level.SetSize(size);
}

void UIProgressBar::Render()
{
    _slider_bg.Render();
    _slider_level.Render();
}


//
// Text Box
//

UITextBox::UITextBox()
{
    _box.SetImage(Storage::IMAGE_DIR / "ui" / "ui_textbox.png");
    _text.SetText("");
    _text.SetColor({0.0f, 0.0f, 0.0f, 1.0f});
    _text.SetFontSize(0.4f);
    _cursor.SetText("1");
    _cursor.SetColor({0.0f, 0.0f, 0.0f, 1.0f});
    _cursor.SetFontSize(0.6f);
    _cursor_time = 0;
}

void UITextBox::SetPosition(glm::vec2 position)
{
    _position = position;
    _box.SetPosition(position);
    _text.SetPosition({position.x + 10, position.y + 10});
    _cursor.SetPosition({position.x + 10, position.y + 10});
}

void UITextBox::SetSize(glm::vec2 size)
{
    _size = size;
    _box.SetSize(size);
}

void UITextBox::SetText(std::string text)
{
    _text.SetText(text);
    _text_str = text;
}

std::string UITextBox::GetText()
{
    return _text_str;
}

void UITextBox::Update(float delta_time, MouseState mouse_state)
{
    // Text
    if (_active && !LAST_INPUT_KEY_HANDLED)
    {
        if (LAST_INPUT_KEY == GLFW_KEY_BACKSPACE)
        {
            _text_str = _text_str.substr(0, _text_str.length() - 1);
        }
        else
        {
            float text_width = UIText::GetTextSizeInPixels(_text_str, 0.4f).x;
            if (text_width < 0.9f * _size.x)
                _text_str += (char)LAST_INPUT_KEY;
        }
        _text.SetText(_text_str);
        LAST_INPUT_KEY_HANDLED = true;
        _cursor_time = 0;
    }
    else if (!_active)
    {
        LAST_INPUT_KEY_HANDLED = true;
    }

    // Cursor
    float new_text_width = UIText::GetTextSizeInPixels(_text_str, 0.4f).x;
    _cursor.SetPosition({_position.x + 10 + new_text_width, _position.y + 10});
    if (_active)
    {
        _cursor_time += delta_time;
        if (_cursor_time > 0.8f)
            _cursor_time = 0;
    }
    else
    {
        _cursor_time = 0;
    }

    bool mouse_on_box = mouse_state.position.x >= _position.x && mouse_state.position.x <= _position.x + _size.x && mouse_state.position.y >= _position.y && mouse_state.position.y <= _position.y + _size.y;
    if (mouse_state.left_clicked)
        _active = mouse_on_box;
}

void UITextBox::Render()
{
    _box.Render();
    _text.Render();

    if (_active && _cursor_time < 0.4f)
        _cursor.Render();
}
